#include "DIRE/Shower/Shower.H"

#include "DIRE/Tools/Amplitude.H"
#include "DIRE/Shower/Cluster.H"
#include "DIRE/Tools/Weight.H"
#include "MODEL/Main/Model_Base.H"
#include "MODEL/Main/Single_Vertex.H"
#include "PDF/Main/ISR_Handler.H"
#include "ATOOLS/Math/Random.H"
#include "ATOOLS/Org/Run_Parameter.H"
#include "ATOOLS/Org/Data_Reader.H"
#include "ATOOLS/Org/My_Limits.H"
#include "ATOOLS/Org/Message.H"

using namespace DIRE;
using namespace MODEL;
using namespace ATOOLS;

Shower::Shower():
  p_model(NULL),
  p_cluster(new Cluster(this))
{
  p_pdf[1]=p_pdf[0]=NULL;
}

Shower::~Shower()
{
  for (Kernel_Vector::const_iterator it(m_cks.begin());
       it!=m_cks.end();++it) delete *it;
  delete p_cluster;
}

struct FTrip {
public:
  Flavour m_a, m_b, m_c;
public:
  inline FTrip(const Flavour &a,const Flavour &b,const Flavour &c):
    m_a(a), m_b(b), m_c(c) {}
  bool operator<(const FTrip &f) const
  {
    if (m_a<f.m_a) return true;
    if (m_a==f.m_a) {
      if (m_b<f.m_b) return true;
      if (m_b==f.m_b) {
	return m_c<f.m_c;
      }
    }
    return false;
  }
};

bool Shower::Init(MODEL::Model_Base *const model,
		  PDF::ISR_Handler *const isr,
		  ATOOLS::Data_Reader *const read)
{
  DEBUG_FUNC(this);
  p_model=model;
  for (int i=0;i<2;++i) p_pdf[i]=isr->PDF(i);
  m_tmin[0]=ToType<double>(rpa->gen.Variable("CSS_FS_PT2MIN"));
  m_tmin[1]=ToType<double>(rpa->gen.Variable("CSS_IS_PT2MIN"));
  m_cplfac[0]=ToType<double>(rpa->gen.Variable("CSS_FS_AS_FAC"));
  m_cplfac[1]=ToType<double>(rpa->gen.Variable("CSS_IS_AS_FAC"));
  m_rsf=ToType<double>(rpa->gen.Variable("RENORMALIZATION_SCALE_FACTOR"));
  m_rcf=read->GetValue<double>("CSS_RECALC_FACTOR",4.0);
  m_tcef=read->GetValue<double>("CSS_TC_ENHANCE",1.0);
  m_kin=read->GetValue<int>("CSS_KIN_SCHEME",1);
  m_kfac=read->GetValue<int>("CSS_KFACTOR_SCHEME",1);
  m_cpl=read->GetValue<int>("CSS_COUPLING_SCHEME",1);
  m_pdfmin=read->GetValue<double>("CSS_PDF_MIN",1.0e-6);
  m_maxem=read->GetValue<unsigned int>
    ("CSS_MAXEM",std::numeric_limits<unsigned int>::max());
  m_oef=read->GetValue<double>("CSS_OEF",3.0);
  if (msg_LevelIsDebugging()) {
    msg_Out()<<METHOD<<"(): {\n\n"
	     <<"   // available gauge calculators\n\n";
    Gauge_Getter::PrintGetterInfo(msg->Out(),25);
    msg_Out()<<"\n   // available lorentz calculators\n\n";
    Lorentz_Getter::PrintGetterInfo(msg->Out(),25);
    msg_Out()<<"\n}"<<std::endl;
  }
  int types(read->GetValue<int>("CSS_KERNEL_TYPE",15));
  std::set<FTrip> sfs;
  const Vertex_Table *vtab(model->VertexTable());
  for (Vertex_Table::const_iterator
	 vlit=vtab->begin();vlit!=vtab->end();++vlit) {
    for (Vertex_List::const_iterator 
	   vit=vlit->second.begin();vit!=vlit->second.end();++vit) {
      Single_Vertex *v(*vit);
      if (v->NLegs()>3) continue;
      if (sfs.find(FTrip(v->in[0],v->in[1],v->in[2]))
	  !=sfs.end()) continue;
      msg_Indent();
      sfs.insert(FTrip(v->in[0],v->in[1],v->in[2]));
      sfs.insert(FTrip(v->in[0],v->in[2],v->in[1]));
      msg_IODebugging()<<"Add "<<v->in[0].Bar()<<" -> "
		       <<v->in[1]<<" "<<v->in[2]<<" {\n";
      if (!(m_kfac&256)) {
	msg_Indent();
	for (int type(0);type<4;++type)
	  if (types&(1<<type))
	    for (int mode(0);mode<2;++mode)
	      for (int swap(0);swap<2;++swap)
		AddKernel(new Kernel(this,Kernel_Key(v,mode,swap,type,read)));
      }
      msg_IODebugging()<<"}\n";
    }
  }
  if (!((m_kfac&2) || (m_kfac&1024)) || (m_kfac&512)) return true;
  ATOOLS::Flavour_Vector fls(4);
  for (long int i(-5);i<=5;++i) {
    if (i==0) continue;
    fls[0]=(fls[2]=Flavour(i)).Bar();
    for (long int j(-5);j<=5;++j) {
      if (j==0 || j==i) continue;
      fls[3]=(fls[1]=Flavour(j)).Bar();
      if (fls[0].Mass() || fls[3].Mass()) continue;
      for (int type(0);type<4;++type)
	if (types&(1<<type))
	  AddKernel(new Kernel(this,Kernel_Key(fls,1,type,read,"FFFF")));
    }
  }
  return true;
}

void Shower::AddKernel(Kernel *const k)
{
  if (k->On()<0) {
    delete k;
    return;
  }
  k->GF()->SetLimits();
  if (k->On()) m_sks[k->LF()->Flav(0)].push_back(k);
  if (k->LF()->Flavs().size()>3) k->SetEF(m_tcef);
  m_cks.push_back(k);
  m_kmap[k->Type()|(k->Type()&1?(k->Mode()?4:0):0)]
    [k->LF()->Flav(1)][k->LF()->Flav(2)]=k;
}


void Shower::SetMS(ATOOLS::Mass_Selector *const ms)
{
  for (Kernel_Vector::const_iterator
	 it(m_cks.begin());it!=m_cks.end();++it)
    (*it)->LF()->SetMS(ms);
}

void Shower::AddWeight(const Amplitude &a,const double &t)
{
  double cw(1.0);
  for (size_t i(0);i<a.size();++i) {
    cw*=a[i]->GetWeight(Max(t,m_tmin[a[i]->Beam()?1:0]));
    a[i]->ClearWeights();
  }
  m_weight*=cw;
  msg_Debugging()<<a<<" t = "<<t<<" -> w = "<<cw
		 <<" ("<<m_weight<<")\n";
}

int Shower::Evolve(Amplitude &a,double &w,unsigned int &nem)
{
  DEBUG_FUNC(this);
  m_weight=1.0;
  msg_Debugging()<<a<<"\n";
  if (nem>=m_maxem) return 1;
  double t(a.T());
  for (Splitting s(GeneratePoint(a,t));
       s.m_t>Max(a.T0(),m_tmin[s.m_type&1]);
       s=GeneratePoint(a,s.m_t)) {
    int stat(s.p_sk->Construct(s,1));
    msg_IODebugging()<<"t = "<<s.m_t<<", stat = "<<stat<<"\n";
    AddWeight(a,s.m_t);
    if (stat==0) return stat;
    if (stat<0) continue;
    if (++nem>=m_maxem) break;
  }
  AddWeight(a,0.0);
  return 1;
}

Splitting Shower::GeneratePoint(const Amplitude &a,const double &t)
{
  Splitting win;
  double tmin[2]={m_tmin[0],m_tmin[1]};
  for (Amplitude::const_reverse_iterator
	 it(a.rbegin());it!=a.rend();++it) {
    Splitting cur(GeneratePoint(**it,t));
    if (cur.p_c==NULL || cur.p_s==NULL) continue;
    if (cur.m_t<m_tmin[cur.m_type&1]) continue;
    m_tmin[0]=m_tmin[1]=(win=cur).m_t;
  }
  m_tmin[0]=tmin[0];
  m_tmin[1]=tmin[1];
  if (win.p_sk && win.m_t>m_tmin[win.m_type&1])
    msg_Debugging()<<"Emission at "<<win<<"\n";
  return win;
}

Splitting Shower::GeneratePoint(Parton &p,const double &t)
{
  Splitting win(&p,NULL,t);
  double sum=0.0, ct=m_rcf*t;
  SKernel_Map::const_iterator kit(m_sks.find(p.Flav()));
  if (kit==m_sks.end()) return Splitting();
  std::vector<Parton_Vector> specs(kit->second.size());
  std::vector<std::vector<double> > psum(kit->second.size());
  std::vector<std::vector<size_t> > splits(psum.size());
  while (true) {
    if (win.m_t*m_rcf<=ct) {
      sum=0.0;
      ct=win.m_t;
      for (size_t j(0);j<kit->second.size();++j) {
	specs[j].clear();
	psum[j].clear();
	splits[j].clear();
	double csum=0.0;
	for (int i(p.Ampl()->size()-1);i>=0;--i) {
	  if ((*p.Ampl())[i]==&p) continue;
	  Splitting cur(&p,(*p.Ampl())[i]);
	  cur.SetType();
	  cur.m_kfac=m_kfac;
	  cur.m_cpl=m_cpl;
	  cur.m_t1=ct;
	  for (cur.m_cm=0;cur.m_cm<2;++cur.m_cm)
	    if (kit->second[j]->Allowed(cur)) {
	      specs[j].push_back(cur.p_s);
	      double I=kit->second[j]->Integral(cur);
	      psum[j].push_back(csum+=dabs(I));
	      splits[j].push_back(i);
	    }
	}
	if (psum[j].size()) sum+=psum[j].back();
      }
      if (sum==0.0) return Splitting();
      win=Splitting(&p,NULL,ct);
    }
    win.m_t*=exp(log(ran->Get())*2.0*M_PI/sum);
    if (win.m_t<m_tmin[p.Beam()?1:0]) return win;
    double disc(sum*ran->Get()), csum(0.0);
    for (size_t j(0);j<splits.size();++j)
      if (splits[j].size() &&
	  (csum+=psum[j].back())>=disc) {
	double disc(psum[j].back()*ran->Get());
	for (size_t i(0);i<splits[j].size();++i)
	  if (psum[j][i]>=disc) {
	    win.p_s=(*p.Ampl())[splits[j][i]];
	    win.SetType();
	    win.m_kin=m_kin;
	    win.m_kfac=m_kfac;
	    win.m_cpl=m_cpl;
	    win.m_t1=ct;
	    if (!kit->second[j]->GeneratePoint(win)) {
	      msg_Error()<<METHOD<<"(): Error generating point!\n";
	      msg_Debugging()<<win<<"\nQ2 = "<<win.m_Q2
			     <<", eta = "<<win.m_eta
			     <<", t0 = "<<win.m_t0<<"\n";
	      break;
	    }
	    if (kit->second[j]->LF()->Construct(win,0)!=1) break;
	    win.m_w=kit->second[j]->GetWeight(win,m_oef);
	    if (win.m_w.MC()<ran->Get()) {
	      win.p_c->AddWeight(win.p_s,win.m_t,win.m_w.Reject());
	      msg_IODebugging()<<"t = "<<win.m_t<<", w = "<<win.m_w.MC()
			       <<" / "<<win.m_w.Reject()<<" -> reject ["
			       <<win.p_c->Id()<<"<->"<<win.p_s->Id()<<"]\n";
	      break;
	    }
	    win.p_c->AddWeight(win.p_s,win.m_t,win.m_w.Accept());
	    msg_IODebugging()<<"t = "<<win.m_t<<", w = "<<win.m_w.MC()
			     <<" / "<<win.m_w.Accept()<<" -> select ["
			     <<win.p_c->Id()<<"<->"<<win.p_s->Id()<<"]\n";
	    win.m_ss=specs[j];
	    return win;
	  }
	break;
      }
  }
  return win;
}

double Shower::GetXPDF
(const double &x,const double &Q2,
 const ATOOLS::Flavour &fl,const int b) const
{
  if (p_pdf[b]==NULL) return 1.0;
  if (!p_pdf[b]->Contains(fl.Bar())) {
    if (fl.Strong() || fl.Mass()<10.0) return 0.0;
    return 1.0;
  }
  if (Q2<sqr(2.0*fl.Mass(true))) return 0.0;
  if (x<p_pdf[b]->XMin() ||
      x>p_pdf[b]->XMax()*p_pdf[b]->RescaleFactor() ||
      Q2<p_pdf[b]->Q2Min() || Q2>p_pdf[b]->Q2Max())
    return 0.0;
  p_pdf[b]->Calculate(x,Q2);
  return p_pdf[b]->GetXPDF(fl.Bar());
}

Kernel *Shower::GetKernel(const Splitting &s,const int mode) const
{
  Kernel_Map::const_iterator seit(m_kmap.find(s.m_type|(mode?4:0)));
  if (seit==m_kmap.end()) return NULL;
  SEKernel_Map::const_iterator eit(seit->second.find(s.p_c->Flav()));
  if (eit==seit->second.end()) return NULL;
  EKernel_Map::const_iterator it(eit->second.find(s.p_n->Flav()));
  if (it==eit->second.end()) return NULL;
  if (s.p_s==NULL || it->second->GF()->Allowed(s)) return it->second;
  return NULL;
}

int Shower::RemnantTest(Parton *const c,const Vec4D &p)
{
  Vec4D pb(rpa->gen.PBeam(c->Beam()-1));
  if (p[0]<0.0 || p.Nan()) return -1;
  if (p[0]>pb[0] && !IsEqual(p[0],pb[0],1.0e-6)) return -1;
  return 1;
}

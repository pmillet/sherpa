#include "SHRiMPS/Eikonals/Omega_ik.H"
#include "SHRiMPS/Tools/MinBias_Parameters.H"
#include "ATOOLS/Math/Random.H"
#include "ATOOLS/Math/Gauss_Integrator.H"
#include "ATOOLS/Org/Run_Parameter.H"
#include "ATOOLS/Org/Message.H"

using namespace SHRIMPS;
using namespace ATOOLS;

Omega_ik::Omega_ik(Form_Factor * ff1,Form_Factor * ff2,
		   const int & Bsteps,const int & test) :
  p_ff1(ff1), p_ff2(ff2), 
  m_lambda(MBpars("lambda")), m_Delta(MBpars("Delta")),
  m_absorp(MBpars.Absorption()),
  m_originalY(MBpars("originalY")), m_cutoffY(MBpars("deltaY")), 
  m_Y(m_originalY-m_cutoffY), m_Ysteps(20.),
  m_Omegaik(Eikonal_Contributor(ff1,ff2)),
  m_Omegaki(Eikonal_Contributor(ff1,ff2)),
  m_Bmin(MBpars("bmin")), m_Bmax(MBpars("bmax")), 
  m_deltaB((m_Bmax-m_Bmin)/double(Bsteps)), 
  m_singletwt(MBpars("SingletWt")),
  m_sigmaInelastic(0.), 
  m_test(test)
{
}

Omega_ik::~Omega_ik() {}

Eikonal_Contributor * Omega_ik::GetSingleTerm(const int & i) {
  if (i==0) return &m_Omegaik;
  if (i==1) return &m_Omegaki;
  msg_Error()<<"Error in "<<METHOD<<"("<<i<<"):"<<std::endl
	     <<"   Out of range.  Will exit the run."<<std::endl;
  exit(1);  
}


double Omega_ik::operator()(const double & B) const {
  if (B<0. || B>=m_Bmax) return 0.;
  size_t Bbin(int(B/m_deltaB));
  return ((m_gridB[Bbin]*((Bbin+1)*m_deltaB-B)+
	   m_gridB[Bbin+1]*(B-Bbin*m_deltaB))/m_deltaB);
}

double Omega_ik::Maximum(const double & B) const {
  if (B<0. || B>=m_Bmax) return 0.;
  size_t Bbin(int(B/m_deltaB));
  return ((m_gridBmax[Bbin]*((Bbin+1)*m_deltaB-B)+
	   m_gridBmax[Bbin+1]*(B-Bbin*m_deltaB))/m_deltaB);
}

ATOOLS::Vec4D Omega_ik::
SelectB1B2(double & b1,double & b2,const double & B) {
  double maxvalue(1.1*Maximum(B));
  double theta(0.),b1max(m_Omegaik.B1max()),value(0.);  
  bool   accept(false);
  while (!accept) {
    theta = 2.*M_PI*ATOOLS::ran->Get();
    b1    = b1max * ATOOLS::ran->Get();
    b2    = sqrt(B*B+b1*b1-2.*B*b1*cos(theta));
    value = b1*m_Omegaik(b1,b2,0.)*m_Omegaki(b1,b2,0.);
    if (value>maxvalue) 
      msg_Error()<<"Warning in "<<METHOD<<"("<<b1<<", "<<b2<<", "<<B<<"):"
		 <<std::endl
		 <<"   Value = "<<value
		 <<" > maxvalue = "<<maxvalue<<"."<<std::endl;
    if (b1<p_ff1->Bmax() && b2<p_ff2->Bmax() &&
	value/maxvalue>ATOOLS::ran->Get()) accept=true;
  }
  return Vec4D(0.,b1*cos(theta),b1*sin(theta),0.);
}

double Omega_ik::
MaximalEmissionProbability(const double & b1,const double & b2) 
{
  return m_Delta;
}


double Omega_ik::
EmissionWeight(const double & b1,const double & b2,const double & y,
	       const double & sup) {
  if (y<-m_originalY || y>m_originalY) return 0.;
  if (y<-m_Y || y>m_Y) return 1.;
  double term1 = ATOOLS::Max(1.e-12,m_lambda/2.*sup*m_Omegaik(b1,b2,y));
  double term2 = ATOOLS::Max(1.e-12,m_lambda/2.*sup*m_Omegaki(b1,b2,y));
  double absorption(1.);
  switch (m_absorp) {
  case absorption::factorial:
    if (!ATOOLS::IsZero(term1) && !ATOOLS::IsZero(term2)) 
      absorption = (1.-exp(-term1))/term1 * (1.-exp(-term2))/term2;
    break;
  case absorption::exponential:
  default:
    absorption = exp(-(term1+term2));
    break;
  }
  return absorption;
}

double Omega_ik::SingletWeight(const double & b1,const double & b2,
			       const double & y1,const double & y2,
			       const double & sup,const int & nbeam) {
  double term   = m_singletwt*DeltaOmega(b1,b2,y1,y2,sup,nbeam); 
  double weight = sqr(1.-exp(-term/2.));
  return weight;
}

double Omega_ik::OctetWeight(const double & b1,const double & b2,
			     const double & y1,const double & y2,
			     const double & sup,const int & nbeam) {
  double term   = DeltaOmega(b1,b2,y1,y2,sup,nbeam); 
  double weight = 1.-exp(-term);
  return weight;
}

double Omega_ik::RescatterProbability(const double & b1,const double & b2,
				      const double & y1,const double & y2,
				      const double & sup,const int & nbeam) {
  double term   = DeltaOmega(b1,b2,y1,y2,sup,nbeam); 
  double weight = 1.-exp(-term);
  return weight;
}

double Omega_ik::DeltaOmega(const double & b1,const double & b2,
			    const double & y1,const double & y2,
			    const double & sup,const int & nbeam) {
  if (dabs(y1)>m_originalY || dabs(y2)>m_originalY) return 0.;
  double meany((y1+y2)/2.), ommaj, ommin;
  if (meany<0.) {
    ommaj = (y1<y2)?m_Omegaik(b1,b2,y2):m_Omegaik(b1,b2,y1);
    ommin = (y1<y2)?m_Omegaik(b1,b2,y1):m_Omegaik(b1,b2,y2);
  }
  else {
    ommaj = (y1<y2)?m_Omegaki(b1,b2,y1):m_Omegaki(b1,b2,y2);
    ommin = (y1<y2)?m_Omegaki(b1,b2,y2):m_Omegaki(b1,b2,y1);
  }
  return sup*pow(m_lambda,2-nbeam)*dabs(ommaj-ommin)/(ommin);
}

double Omega_ik::Sum(const double & b1,const double & b2,const double & y){
  if (y<-m_originalY || y>m_originalY) return 0.;
  if (y<-m_Y || y>m_Y) return 1.;
  double term1 = m_Omegaik(b1,b2,y)/p_ff1->FourierTransform(b1);
  double term2 = m_Omegaki(b1,b2,y)/p_ff2->FourierTransform(b2);

  return term1+term2;
}

void Omega_ik::PrepareQT(const double & b1,const double & b2) {  
  double D1,D2,invD,y;
  m_Omegaik.SetB1B2(b1,b2);
  m_Omegaki.SetB1B2(b1,b2);
  Gauss_Integrator inti(&m_Omegaik), intk(&m_Omegaki);
  m_gridD.clear();
  for (int i=0;i<=m_Ysteps;i++) {
    y    = m_Y*(1.-2.*double(i)/m_Ysteps);
    D1   = inti.Integrate(-m_Y,y,2.e-2,1);
    D1  += intk.Integrate(-m_Y,y,2.e-2,1);
    D2   = inti.Integrate(y,m_Y,2.e-2,1);
    D2  += intk.Integrate(y,m_Y,2.e-2,1);
    invD = 1./D1+1./D2;
    m_gridD.push_back(invD);
  }
}

double Omega_ik::EffectiveIntercept(double b1,double b2,const double & y)
{
  if (b1<0.) b1 = m_Bmax;
  if (b2<0.) b2 = m_Bmax;
  return m_Delta*exp(-m_lambda*(m_Omegaik(b1,b2,y)+m_Omegaki(b1,b2,y))/2.);
}

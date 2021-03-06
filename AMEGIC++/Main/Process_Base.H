#ifndef AMEGIC_Main_Process_Base_H
#define AMEGIC_Main_Process_Base_H

#include "AMEGIC++/Main/Polarisation.H"
#include "AMEGIC++/Main/Amegic_Base.H"
#include "AMEGIC++/Amplitude/Vertex.H"
#include "PHASIC++/Process/Single_Process.H"

namespace MODEL { class Model_Base; }

namespace AMEGIC {

  class Amplitude_Handler;

  class Process_Base : public PHASIC::Single_Process, public Amegic_Base {
  protected:

    MODEL::Model_Base    *p_model;

    std::string m_resdir;

    int *p_b, m_eoreset, m_allowmap;

    AMEGIC::Pol_Info      *p_pl;
    
    std::string m_print_graphs;

    std::list<std::string>               *p_channellibnames;

    ATOOLS::Vec4D* p_testmoms;

    double         m_Norm;
    double         m_sfactor, m_lastdxs;

    std::map<ATOOLS::Flavour,ATOOLS::Flavour> m_eflmap;
    std::map<std::string,ATOOLS::Flavour> m_fmap;

    ATOOLS::NLO_subevtlist* p_subevtlist;

    static int s_gauge;

  public:

    // constructor
    Process_Base();

    // destructor
    virtual ~Process_Base();

    // member functions
    void Init();
    virtual std::string CreateLibName();
    virtual int  InitAmplitude(Amegic_Model *,Topology *,
			       std::vector<Process_Base *> &,
			       std::vector<Process_Base *> &) = 0;

    virtual bool SetUpIntegrator() = 0; 
    virtual void AddChannels(std::list<std::string>*) = 0;
    virtual Amplitude_Handler *GetAmplitudeHandler() = 0;   
    virtual Helicity *GetHelicity() = 0;
    virtual Process_Base *Partner() const = 0;
    virtual std::string PSLibName() = 0;

    virtual void FillAlphaHistogram(ATOOLS::Histogram*,double) {}

    virtual double Result();
    virtual void SetPrintGraphs(std::string gpath="");
    virtual std::string LibName();

    virtual int                             NumberOfDiagrams() = 0;
    virtual AMEGIC::Point                 * Diagram(int i) = 0;
    virtual bool IsFreeOfFourVertex(AMEGIC::Point * _p) { return 1; }

    bool FillIntegrator(PHASIC::Phase_Space_Handler *const psh);

    // inline functions
    inline std::list<std::string> *GetChannelLibNames() const 
    { return p_channellibnames; }

    void SetTestMoms(ATOOLS::Vec4D* mom)  { p_testmoms=mom; }
    ATOOLS::Vec4D*  GetTestMoms()         { return p_testmoms; }
    
    int  OSDecays() { return p_pinfo->OSDecays(); }
    virtual int Type()  { return 0; }
    double Norm()       { return m_Norm; }
    void   SetNorm(double norm) { m_Norm=norm; }
    double GetSFactor() { return m_sfactor; }
    double LastDXS() { return m_lastdxs; }
    virtual double operator()(const ATOOLS::Vec4D *) { return 0.; }
    virtual void               PrintProcessSummary(int=0);
    ATOOLS::Flavour ReMap(const ATOOLS::Flavour&,const std::string&) const;
    ATOOLS::Flavour ReMap(const ATOOLS::Flavour &fl,const size_t &id) const;
    inline static void SetGauge(const int gauge) { s_gauge=gauge; }
    inline void SetSubevtList(ATOOLS::NLO_subevtlist *subs) { p_subevtlist=subs; }
    ATOOLS::NLO_subevtlist *GetRSSubevtList() { return p_subevtlist; }

    virtual Process_Base *GetReal();

    inline int EOReset() const { return m_eoreset; }
  protected:
    double         SymmetryFactors();
    double         SBSymmetryFactor(ATOOLS::Flavour* flout,size_t nout);
    
    bool           CheckMapping(const Process_Base * proc);
    bool           FlavCompare(PHASIC::Process_Base *const proc);
    void           InitFlavmap(const Process_Base * proc);
    void           AddtoFlavmap(const std::string&,const ATOOLS::Flavour&);

  };

}

#endif


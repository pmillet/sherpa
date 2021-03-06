#ifndef Analysis_Observables_Four_Particle_Observables_H
#define Analysis_Observables_Four_Particle_Observables_H

#include "AddOns/Analysis/Observables/Primitive_Observable_Base.H"

namespace ANALYSIS {

  class Four_Particle_Observable_Base : public Primitive_Observable_Base {  
  protected:
    bool f_special;
    std::vector<ATOOLS::Flavour> m_flavs;
    void virtual Evaluate(const ATOOLS::Vec4D &, const ATOOLS::Vec4D &, 
			  const ATOOLS::Vec4D &, const ATOOLS::Vec4D &,
			  double=1.,double=1.0) =0; 
    void virtual Evaluate(double,double=1.,double=1.0); 
  public:
    Four_Particle_Observable_Base(const std::vector<ATOOLS::Flavour> &,
				  int,double,double,int,
				  const std::string& listname,
				  const std::string& name);
    void virtual Evaluate(int,const ATOOLS::Vec4D *,
			  const ATOOLS::Flavour *,double=1.,double=1.0);
    void virtual Evaluate(const ATOOLS::Particle_List &,
			  double=1.,double=1.0);
  };

  //==========================================================================

  class Four_Particle_PlaneAngle : public Four_Particle_Observable_Base {  
    void Evaluate(const ATOOLS::Vec4D &, const ATOOLS::Vec4D &, 
		  const ATOOLS::Vec4D &, const ATOOLS::Vec4D &,
		  double=1.,double=1.0); 
  public:
    Four_Particle_PlaneAngle(const std::vector<ATOOLS::Flavour>&,
			     int,double,double,int,
			     const std::string& listname);//=std::string("NRAngle:"));
    Primitive_Observable_Base* Copy() const;
  };

  class Four_Particle_PT : public Four_Particle_Observable_Base {  
    void Evaluate(const ATOOLS::Vec4D &, const ATOOLS::Vec4D &, 
		  const ATOOLS::Vec4D &, const ATOOLS::Vec4D &,
		  double=1.,double=1.0);
  public:
    Four_Particle_PT(const std::vector<ATOOLS::Flavour>&,
		     int,double,double,int,
		     const std::string& listname);
    Primitive_Observable_Base* Copy() const;
  };

  class Two_Partonpair_PTdiff : public Four_Particle_Observable_Base {  
    void Evaluate(const ATOOLS::Vec4D&, const ATOOLS::Vec4D&,
		  const ATOOLS::Vec4D&, const ATOOLS::Vec4D&,
		  double=1.,double=1.0);
  public:
    Two_Partonpair_PTdiff(const std::vector<ATOOLS::Flavour>&,
			  int, double, double, int,
			  const std::string& listname);
    Primitive_Observable_Base* Copy() const;
  };

  class Two_Partonpair_Theta : public Four_Particle_Observable_Base {  
    void Evaluate(const ATOOLS::Vec4D&, const ATOOLS::Vec4D&,
		  const ATOOLS::Vec4D&, const ATOOLS::Vec4D&,
		  double=1.,double=1.0);
  public:
    Two_Partonpair_Theta(const std::vector<ATOOLS::Flavour>&,
			 int, double, double, int,
			 const std::string& listname);
    Primitive_Observable_Base* Copy() const;
  };

  // ======================================================================

  class Di_Mass : public Primitive_Observable_Base {
  public:
    Di_Mass(unsigned int type,double xmin,double xmax,int nbins,
	   const std::string & =std::string("Jets"));
    void Evaluate(const ATOOLS::Blob_List & blobs,double value, double);
    Primitive_Observable_Base * Copy() const ;
  };

  // =====================================================================

  class Four_Particle_PlaneAngleCMS : public Four_Particle_Observable_Base {
    void Evaluate(const ATOOLS::Vec4D &, const ATOOLS::Vec4D &, 
                  const ATOOLS::Vec4D &, const ATOOLS::Vec4D &,
                  double=1.,double=1.0); 
  public:
    Four_Particle_PlaneAngleCMS(const std::vector<ATOOLS::Flavour>&,
                             int,double,double,int,
                             const std::string& listname);//=std::string("NRAngle:"));
    Primitive_Observable_Base* Copy() const;
  };

  // =====================================================================

  class Four_Particle_EnergyCMS : public Four_Particle_Observable_Base {
    void Evaluate(const ATOOLS::Vec4D &, const ATOOLS::Vec4D &, 
                  const ATOOLS::Vec4D &, const ATOOLS::Vec4D &,
                  double=1.,double=1.0); 
  public:
    Four_Particle_EnergyCMS(const std::vector<ATOOLS::Flavour>&,
                             int,double,double,int,
                             const std::string& listname);//=std::string("NRAngle:"));
    Primitive_Observable_Base* Copy() const;
  };

  // =====================================================================

  class Four_Particle_Mass : public Four_Particle_Observable_Base {
    void Evaluate(const ATOOLS::Vec4D &, const ATOOLS::Vec4D &,
                  const ATOOLS::Vec4D &, const ATOOLS::Vec4D &,
                  double=1.,double=1.);
  public:
    Four_Particle_Mass(const std::vector<ATOOLS::Flavour>&,
                       int,double,double,int,
                       const std::string& listname);
    Primitive_Observable_Base* Copy() const;
  };

}
#endif

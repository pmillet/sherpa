#ifndef SHRIMPS_Beam_Remnants_Beam_Remnant_Handler_H
#define SHRIMPS_Beam_Remnants_Beam_Remnant_Handler_H

#include "SHRiMPS/Beam_Remnants/Colour_Generator.H"
#include "SHRiMPS/Beam_Remnants/Colour_Reconnections.H"
#include "SHRiMPS/Beam_Remnants/Hadron_Dissociation.H"
#include "SHRiMPS/Beam_Remnants/Continued_PDF.H"
#include "SHRiMPS/Event_Generation/Ladder.H"
#include "SHRiMPS/Eikonals/Omega_ik.H"
#include "ATOOLS/Phys/Blob_List.H"
#include "ATOOLS/Org/Return_Value.H"
#include "ATOOLS/Org/CXXFLAGS.H"
#include <vector>

namespace BEAM   { class Beam_Spectra_Handler; }

namespace SHRIMPS {

  class Beam_Remnant_Handler {
  private:
    ATOOLS::Blob                     * p_blob;
    std::vector<Continued_PDF>       * p_pdfs;
    std::vector<BEAM::Beam_Base *>     m_beams;
    std::vector<ATOOLS::Vec4D>         m_beamvecs;
    std::vector<Hadron_Dissociation *> m_hadrons;
    Colour_Generator                 * p_colour;
    Colour_Reconnections             * p_reconnections;
    std::vector<ATOOLS::Vec4D>         m_checkmom;
    
    Omega_ik         * p_eikonal;
    ATOOLS::Vec4D      m_beamvec[2];
    ATOOLS::Particle * p_part[2];
    double             m_shat;
    size_t             m_paircounter;

    bool m_didinsertbeamblob;

    bool                                        m_analyse;
    std::map<std::string, ATOOLS::Histogram * > m_histomap;
    long int                                    m_momconserv, m_colours;

    void LinkShowerInitiators(ATOOLS::Blob * blob);
    void AddSpectators();
  public:
    Beam_Remnant_Handler() {};
    Beam_Remnant_Handler(BEAM::Beam_Spectra_Handler * beamspectra,
			 std::vector<Continued_PDF> & pdfs);
    ~Beam_Remnant_Handler();

    void Reset(const size_t & mode=0);
    bool InitialiseCollision(const int & N=0, double B=0., 
			     Omega_ik * eikonal=NULL);
    bool NextIS(ATOOLS::Particle *& part1,ATOOLS::Particle *& part2);
    bool UpdateColours(Ladder * ladder,const bool & lastladder);
    ATOOLS::Return_Value::code FillBeamBlobs(ATOOLS::Blob_List * blobs,
					     Omega_ik * eikonal,
					     const double & smin);
    void AddBeamBlobs(ATOOLS::Blob_List * blobs);

    inline Continued_PDF * GetPDF(const int & beam) {
      if (beam!=0 && beam!=1) return NULL;
      return &(*p_pdfs)[beam];
    }
    
    inline void SetBeamBlob(ATOOLS::Blob *const beamblob,const int & beam) {
      m_hadrons[beam]->SetBeamBlob(beamblob);
    } 
    inline void SetInitials(ATOOLS::Particle * part1,ATOOLS::Particle * part2) {
      p_colour->SetInitials(part1,part2);
    }
    inline ATOOLS::Blob * GetCompensatorBlob() const { 
      return p_colour->GetCompensatorBlob();
    }
    inline ATOOLS::Blob * GetSoftColourBlob() const { return p_blob; }
    inline bool FindInTreatedBlobs(ATOOLS::Blob * blob) { return true; }
  };
}
#endif

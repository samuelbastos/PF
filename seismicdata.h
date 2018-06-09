// Projeto Geresim
// Grupo de Visualizacao
//
// tmt_br@tecgraf.puc-rio.br
//
// Tecgraf/PUC-Rio
#ifndef RES_SEISMIC_DATA_H
#define RES_SEISMIC_DATA_H

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <glm\glm.hpp>

/**
 * Class representing Seismic Data
 */
class ResSeismicData
{
public:
  // Default constructor
  ResSeismicData (const char* filename);
  // Destructor
  virtual ~ResSeismicData();

  // Reference Counting
  void IncRef ();
  void DecRef ();
  
  // Returns the path to the associated seismic file
  const char* GetFilename () const;

  // Returns the texture dimensions
  void GetTextureDimensions (int* width, int* height, int* depth) const;

  // Returns the depth limits in meters
  void GetDepthLimits (float* min, float* max) const;

  // Returns Min/Max recorded values
  float GetMinValue () const;
  float GetMaxValue () const;

  // Returns whether the data has been correctly loaded and is valid
  bool IsValid () const;

  // Sets/Gets the Local to World coords displacement
  void SetDisplacement (glm::vec3 displacement);
  glm::vec3 GetDisplacement () const;
  
  // Converts a given point to world/local coordinates
  glm::vec3 ConvertToWorld (glm::vec3 p) const;
  glm::vec3 ConvertToLocal (glm::vec3 p) const;

  // Getters for the box position 
  const glm::vec3& GetBoxPos () const;
  glm::vec3 GetBoxCenter () const;
  glm::vec3 GetBoxDiagonal () const;
  void GetBoxPoints(std::vector<glm::vec3>& vec) const;

  // Getters for the orthogonal base (normalizados)
  glm::vec3 GetBoxDirX () const;
  glm::vec3 GetBoxDirY () const;
  glm::vec3 GetBoxDirZ () const;
  
  // Returns a (xlen, ylen, zlen) vector containing the dimensions of the projection 
  // of each box axis to the world axis counterparts. That is, xlen is the length of
  // the GetBoxDirX vector projected over the (1,0,0) axis, and so on.
  // The returned axis can be used to calculate an AABB for the seismic data.
  glm::vec3 GetBoxAxis () const;

  // Returns 'true' if point 'p' is inside the seismic volume, 'false' otherwise. 'p' should be
  // a point in seismic local coordinates.
  bool InsideVolume (const glm::vec3& p) const;

  // Returns the unit for the Z values
  const char* GetZUnit () const;

  // If texel 'i' holds a valid value (not outside original data bounds), returns 'true' and the normalized value in '*v'.
  // Returns 'false' otherwise.
  bool GetNormalizedValue (int i, float* v) const
  {
    unsigned char texel = m_tex[i];
    if (texel == 0)
      return false;
    *v = (texel-1)/254.0f; // undo transformation done in 'StoredValue'.
    return true;
  }

  unsigned char* getData() const
  {
	  return m_tex;
  }

private:
  // Union for Value Type
  union VType {
    unsigned char b;
    uint16_t s;
    uint32_t i;
    float f;
  };

  // Helper Class for Buffered Reading
  template<class T> class Buffer
  {
  public:
    Buffer (FILE* file, size_t size, size_t block, size_t total);
    ~Buffer ();
    T* NextBlock ();
  private:
    FILE* m_file;
    T* m_buffer;
    size_t m_size;
    size_t m_block;
    size_t m_pending;
    size_t m_total_size;
    size_t m_read_pos;
  };
  
  // .DAT Loader Methods
  bool LoadHeader (FILE* fp);
  bool LoadValues (FILE* fp);
  int SizeOfValue () const;

  // Algebric Helpers
  void GetBoxDeltas (glm::vec3& dx, glm::vec3& dy, glm::vec3& dz) const;


  // Computes the stored texel value from a given normalized value ([0,1]).
  static unsigned char StoredValue (float normalized_value)
  {
    return 1 + (unsigned char)(normalized_value*254); // 0 will hold values outside original data bounds. see 'GetNormalizedValue'.
  }

private:
  int m_ref_count;                 // reference 
  char* m_filename;                // seismic filename
  bool m_updated;                  // if texture object is updated
  char m_vtype;                    // type of value in file (1=8bits, 2=16bits, 3=32bits,4=float)
  int m_ntrace;                    // number of traces
  int m_nx, m_ny, m_nz;            // data dimension
  float m_dz;                      // z interval
  float *m_x, *m_y, *m_z;          // trace top coordinate
  VType m_vmin, m_vmax;            // min/max field values
  unsigned char m_zunit;           // axis z unit
  glm::vec3 m_displacement;        // local to world displacement
  glm::vec3 m_u, m_v, m_w;         // orthogonal (?) base
  glm::vec3 m_pos;                 // box origin
  glm::vec3 m_axis;                // box axis	
  int m_width, m_height, m_depth;  // texture dimension
  unsigned char* m_tex;            // texture image
  float m_sx, m_sy, m_sz;          // texture coord scale
};


#endif // RES_SEISMIC_DATA_H

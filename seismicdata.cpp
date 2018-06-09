// Projeto Geresim
// Grupo de Visualizacao
//
// tmt_br@tecgraf.puc-rio.br
//
// Tecgraf/PUC-Rio

#include "seismicdata.h"

/****************************************************************
 * Static Methods
 ****************************************************************/
#define HEADER_SIZE 17 // bytes

#ifdef WIN32
# if (_MSC_VER >= 1400) // vc8 or later
#  define HAS_64_FUNCTIONS 1
# endif
#endif

static bool Skip (FILE* fp, size_t offset)
{
# ifdef HAS_64_FUNCTIONS
  return _fseeki64(fp, (long long)offset, SEEK_CUR) == 0;
# else
  return fseek(fp, (long)offset, SEEK_CUR) == 0;
# endif
}

static int GetNearestPowerOfTwo (int n)
{
  int d = 2;
  while (d < n)
    d *= 2;
  return d;
}

template<typename T> inline T clamp(T v, T lower, T upper)
{
  return (v) < (lower) ? (lower) : ((v) > (upper) ? (upper) : (v));
}

/****************************************************************
 * Public Methods
 ****************************************************************/

ResSeismicData::ResSeismicData (const char* filename)
: m_ref_count(1)
, m_filename(NULL)
, m_updated(false)
, m_vtype(0)
, m_ntrace(0)
, m_nx(0), m_ny(0), m_nz(0)
, m_dz(0.0f)
, m_x(0), m_y(0), m_z(0)
, m_vmin(), m_vmax()
, m_zunit(0)
, m_u(1.0f, 0.0f, 0.0f)
, m_v(0.0f, 1.0f, 0.0f)
, m_w(0.0f, 0.0f, 1.0f)
, m_pos(0.0f, 0.0f, 0.0f)
, m_axis(0.0f, 0.0f, 0.0f)
, m_displacement(0.0f, 0.0f, 0.0f)
, m_width(0), m_height(0)
, m_depth(0)
, m_tex(NULL)
{
  //utlStringCopy(m_filename, filename);
  FILE* fp = fopen(filename,"rb");
  if (!fp) return;
  
  if (LoadHeader(fp))
	  LoadValues(fp);

  fclose(fp);
}

void ResSeismicData::IncRef ()
{
  m_ref_count++;
}

void ResSeismicData::DecRef ()
{
  m_ref_count--;
  if (m_ref_count == 0)
    delete this;
}

const char* ResSeismicData::GetFilename () const
{
  return m_filename;
}

void ResSeismicData::GetTextureDimensions (int* ni, int* nj, int* nk) const
{
  if (ni) *ni = m_width;
  if (nj) *nj = m_height;
  if (nk) *nk = m_depth;
}

void ResSeismicData::GetDepthLimits (float* min, float* max) const
{
  glm::vec3 displacement = GetDisplacement();
  glm::vec3 pos = GetBoxPos();
  glm::vec3 diag = GetBoxDiagonal();

  float z1 = -fabs(pos.z - displacement.z);
  float z2 = z1 + diag.z;

  if (min) *min = z1 < z2 ? z1 : z2;
  if (max) *max = z1 > z2 ? z1 : z2;
}

float ResSeismicData::GetMinValue () const
{
  switch (m_vtype) {
  case 1: return (float) m_vmin.b;
  case 2: return (float) m_vmin.s;
  case 3: return (float) m_vmin.i;
  case 4: return m_vmin.f;
  }
  return 0.0f;
}

float ResSeismicData::GetMaxValue () const
{
  switch (m_vtype) {
  case 1: return (float) m_vmax.b;
  case 2: return (float) m_vmax.s;
  case 3: return (float) m_vmax.i;
  case 4: return m_vmax.f;
  }
  return 0.0f;
}

bool ResSeismicData::IsValid () const
{
  return m_tex != NULL;
}

void ResSeismicData::SetDisplacement (glm::vec3 displacement)
{
	glm::vec3 dif = m_displacement - displacement;
	m_pos += dif;
	m_displacement = displacement;
}

glm::vec3 ResSeismicData::GetDisplacement () const
{
  return m_displacement;
}

glm::vec3 ResSeismicData::ConvertToWorld (glm::vec3 p) const
{
	return p + m_displacement;
}

glm::vec3 ResSeismicData::ConvertToLocal (glm::vec3 p) const
{
	return p - m_displacement;
}

const glm::vec3& ResSeismicData::GetBoxPos () const
{
  return m_pos;
}

glm::vec3 ResSeismicData::GetBoxCenter () const
{
  glm::vec3 out;
  out.x = m_pos.x+(m_axis.x*m_u.x+m_axis.y*m_v.x+m_axis.z*m_w.x)/2;
  out.y = m_pos.y+(m_axis.x*m_u.y+m_axis.y*m_v.y+m_axis.z*m_w.y)/2;
  out.z = m_pos.z+(m_axis.x*m_u.z+m_axis.y*m_v.z+m_axis.z*m_w.z)/2;
  return out;
}

glm::vec3 ResSeismicData::GetBoxDiagonal () const
{
  if (!IsValid()) return glm::vec3(0,0,0);

  glm::vec3 dx, dy, dz;
  GetBoxDeltas(dx, dy, dz);
  
  return dx + dy + dz;
}

void ResSeismicData::GetBoxPoints(std::vector<glm::vec3>& vec) const
{
  if (!IsValid()) return;

  glm::vec3 dx, dy, dz;
  GetBoxDeltas(dx, dy, dz);

  glm::vec3 pos = GetBoxPos();

  glm::vec3 p0(pos.x,                pos.y,                pos.z);
  glm::vec3 p1(pos.x+dx.x,           pos.y+dx.y,           pos.z+dx.z);
  glm::vec3 p2(pos.x+dx.x+dz.x,      pos.y+dx.y+dz.y,      pos.z+dx.z+dz.z);
  glm::vec3 p3(pos.x+dz.x,           pos.y+dz.y,           pos.z+dz.z);
  glm::vec3 p4(pos.x+dy.x,           pos.y+dy.y,           pos.z+dy.z);
  glm::vec3 p5(pos.x+dx.x+dy.x,      pos.y+dx.y+dy.y,      pos.z+dx.z+dy.z);
  glm::vec3 p6(pos.x+dx.x+dz.x+dy.x, pos.y+dx.y+dz.y+dy.y, pos.z+dx.z+dz.z+dy.z);
  glm::vec3 p7(pos.x+dz.x+dy.x,      pos.y+dz.y+dy.y,      pos.z+dz.z+dy.z);

  vec.emplace_back(p0);
  vec.emplace_back(p1);
  vec.emplace_back(p2);
  vec.emplace_back(p3);
  vec.emplace_back(p4);
  vec.emplace_back(p5);
  vec.emplace_back(p6);
  vec.emplace_back(p7);

}

glm::vec3 ResSeismicData::GetBoxDirX () const
{
  return m_u;
}

glm::vec3 ResSeismicData::GetBoxDirY () const
{
  return m_v;
}

glm::vec3 ResSeismicData::GetBoxDirZ () const
{
  return m_w;
}

glm::vec3 ResSeismicData::GetBoxAxis () const
{
  return m_axis;
}

bool ResSeismicData::InsideVolume (const glm::vec3& p) const
{
	glm::vec3 p_local = p - GetBoxPos();

	glm::vec3 dx, dy, dz;
  GetBoxDeltas(dx, dy, dz);
  glm::vec3 dirx = dx;
  dirx = glm::normalize(dirx);

  float projx = glm::dot(p_local,dirx);
  if (projx < 0 || projx > glm::length(dx))
    return false;
  glm::vec3 diry = dy;
  diry = glm::normalize(diry);
	float projy = glm::dot(p_local,diry);
  if (projy < 0 || projy > glm::length(dy))
    return false;
  glm::vec3 dirz = dz;
  dirz = glm::normalize(dirz);
  float projz = glm::dot(p_local, dirz);
  return 0 <= projz && projz <= glm::length(dz);
}

const char* ResSeismicData::GetZUnit () const
{
  // 0 = Unknown 1 = Pascal (Pa) 2 = Volts (V) 3 = Millivolts (mV) 4 = Amperes (A) 
  // 5 = Meters (m) 6 = Meters per second (m/s) 7 = Meters per second squared (m/s2) 8 = Newton (N) 9 = Watt (W)
  switch (m_zunit) {
    case 1:  return "Pa";
    case 2:  return "V";
    case 3:  return "mV";
    case 4:  return "A";
    case 5:  return "m";
    case 6:  return "m/s";
    case 7:  return "m/s2";
    case 8:  return "N";
    case 9:  return "W";
    default: return "Unknown";
  }
}



/****************************************************************
 * Private Methods
 ****************************************************************/

ResSeismicData::~ResSeismicData ()
{
  free(m_filename);
  free(m_x);
  free(m_y);
  free(m_z);
}

bool ResSeismicData::LoadHeader (FILE* fp)
{
  int version;
  fseek(fp,0,SEEK_SET);
  if ((fread(&version,sizeof(int),1,fp) != 1) ||
      (fread(&m_ntrace,sizeof(int),1,fp) != 1) ||
      (fread(&m_nz,sizeof(int),1,fp) != 1)  ||
      (fread(&m_dz,sizeof(float),1,fp) != 1) ||
      (fread(&m_vtype,sizeof(char),1,fp) != 1) 
     ) {
    fclose(fp);
    return false;
  }
  m_dz /= 1000;

#if 0
  printf("ntrace nz dz vtype: %d %d %f %d\n",m_ntrace,m_nz,m_dz,m_vtype);
#endif
  
  // load x,y,z vectors
  (void)Skip(fp, (size_t)m_ntrace*m_nz*SizeOfValue()); // skip trace values

  m_x = (float*) malloc(m_ntrace*sizeof(float));
  m_y = (float*) malloc(m_ntrace*sizeof(float));
  m_z = (float*) malloc(m_ntrace*sizeof(float));
  if ((fread(m_x,sizeof(float),m_ntrace,fp) != m_ntrace) ||
      (fread(m_y,sizeof(float),m_ntrace,fp) != m_ntrace) ||
      (fread(m_z,sizeof(float),m_ntrace,fp) != m_ntrace)
     ) {
    return false;
  }
  for (int i=0; i<m_ntrace; ++i) {
    m_z[i] = -m_z[i];
    //printf("%f %f %f\n",m_x[i],m_y[i],m_z[i]);
  }
  
  // read min/max values
  switch (m_vtype) {
    case 1: {
      unsigned char vmin, vmax;
      if ((fread(&vmin,sizeof(vmin),1,fp) != 1) ||
          (fread(&vmax,sizeof(vmax),1,fp) != 1)
         ) {
        return false;
      }
      m_vmin.b = vmin;
      m_vmax.b = vmax;
      //printf("min max: %d %d\n",vmin,vmax);
    }
    break;
    case 2: {
      uint16_t vmin, vmax;
      if ((fread(&vmin,sizeof(vmin),1,fp) != 1) ||
          (fread(&vmax,sizeof(vmax),1,fp) != 1)
         ) {
        return false;
      }
      m_vmin.s = vmin;
      m_vmax.s = vmax;
      //printf("min max: %d %d\n",vmin,vmax);
    }
    break;
    case 3: {
      uint32_t vmin, vmax;
      if ((fread(&vmin,sizeof(vmin),1,fp) != 1) ||
          (fread(&vmax,sizeof(vmax),1,fp) != 1)
         ) {
        return false;
      }
      m_vmin.i = vmin;
      m_vmax.i = vmax;
      //printf("min max: %d %d\n",vmin,vmax);
    }
    break;
    case 4: {
      float vmin, vmax;
      if ((fread(&vmin,sizeof(vmin),1,fp) != 1) ||
          (fread(&vmax,sizeof(vmax),1,fp) != 1)
         ) {
        return false;
      }
      m_vmin.f = vmin;
      m_vmax.f = vmax;
      //printf("min max: %f %f\n",vmin,vmax);
    }
    break;
    default:
    return false;
  }
  // read z unit
  if (fread(&m_zunit,sizeof(m_zunit),1,fp) != 1) 
    return false;
  //printf("zunit: %d\n",m_zunit);

  // find out data dimension
  m_nx = 1;
  glm::vec3 v(m_x[1] - m_x[0],m_y[1] - m_y[0],m_z[1] - m_z[0]);
  for (int i=2; i<m_ntrace; ++i) {
	  glm::vec3 u(m_x[i] - m_x[i-1], m_y[i] - m_y[i-1], m_z[i] - m_z[i-1]);
    if (glm::dot(v,u) < 0) { // dot product
      m_nx = i;
      break;
    }
  }
  m_ny = m_ntrace/m_nx;
  
  // find out orthogonal base
  if (m_nx > 1)
  {
	  m_u.x = m_x[m_nx - 1] - m_x[0];
	  m_u.y = m_y[m_nx - 1] - m_y[0];
	  m_u.z = m_z[m_nx - 1] - m_z[0];
  }
  if (m_ny > 1)
  {
	  m_v.x = m_x[m_ntrace - m_nx] - m_x[0];
	  m_v.y = m_y[m_ntrace - m_nx] - m_y[0];
	  m_v.z = m_z[m_ntrace - m_nx] - m_z[0];
  }
 
  m_axis = glm::vec3(m_u.x,m_v.y,m_nz*m_dz);
  m_u = glm::normalize(m_u);
  m_v = glm::normalize(m_v);
  m_w = glm::cross(m_u, m_v);

  // Since m_u and m_w are expected to be orthogonal, there's no need 
  // to normalize m_w. We simply ensure it is pointing downwards.
  if (m_w.z > 0)
  {
	  m_w.x *= -1;
	  m_w.y *= -1;
	  m_w.z *= -1;
  }
  m_displacement.x = m_x[0]-m_w.x*m_axis.z;
  m_displacement.y = m_y[0]-m_w.y*m_axis.z;
  m_displacement.z = m_z[0]+m_w.z*m_axis.z;

#if 0
  printf("dim: %d %d %d\n",m_nx,m_ny,m_nz);
  printf("u: %f %f %f\n",m_u.x,m_u.y,m_u.z);
  printf("v: %f %f %f\n",m_v.x,m_v.y,m_v.z);
  printf("w: %f %f %f\n",m_w.x,m_w.y,m_w.z);
  printf("is ortho: %f %f %f\n",m_u.Dot(m_v),m_u.Dot(m_w),m_v.Dot(m_w));
  printf("pos: %f %f %f\n",m_pos.x,m_pos.y,m_pos.z);
  printf("displacement: %f %f %f\n",m_displacement.x,m_displacement.y,m_displacement.z);
  printf("axis: %f %f %f\n",m_axis.x,m_axis.y,m_axis.z);
#endif
  return true;
}

bool ResSeismicData::LoadValues (FILE* fp)
{
  int report_interval = (int) fmax(m_ntrace / 100.0f, 1);
  m_width = m_nx;
  m_height = m_ny;
  m_depth = m_nz;

  // limit size to maximun allowed
  int fw = 1, fh = 1, fd = 1;
  int maxsize = 512;
  while (m_width/fw > maxsize)
    fw++;
  while (m_height/fh > maxsize)
    fh++;
  while (m_depth/fd > maxsize)
    fd++;

  m_width = m_width / fw;
  m_height = m_height / fh;
  m_depth = m_depth / fd;
  
  int tex_size = m_width*m_height*m_depth;
  float* normalized_values = (float*) calloc(tex_size, sizeof(float));
  if (!normalized_values)
    return false;
  unsigned char*  n_values  = (unsigned char*)  calloc(tex_size, sizeof(unsigned char));
  if (!n_values) {
    free(normalized_values);
    return false;
  }

  fseek(fp,HEADER_SIZE,SEEK_SET);  // skip dimensions
  // read trace values
  size_t buffer_size = (size_t) fmax(3 * 1024 * 1024, m_nz);
  size_t total_size = (size_t)m_nz * m_ntrace;
  
  switch (m_vtype) {
    case 1: {
      Buffer<unsigned char> buffer(fp, buffer_size, m_nz, total_size);
      for (int t=0; t<m_ntrace; ++t) {
        int i = clamp((t % m_nx) / fw, 0, m_width - 1);
        int j = clamp((t / m_nx) / fh, 0, m_height - 1);

        unsigned char* trace = buffer.NextBlock();
        if (trace == NULL) {
          free(normalized_values);
          free(n_values);
          return false;
        }

        for (int k=0; k<m_nz; ++k) {
          int kk = clamp((m_nz - 1 - k) / fd, 0, m_depth - 1);
          int index = clamp(m_width * (kk*m_height + j) + i, 0, tex_size - 1);
          if (n_values[index] < 255) { // we use an unsigned char to store this number (it will probably not exceed this limit)
            normalized_values[index] += trace[k] / 255.0f;
            n_values[index]++;
          }
        }
      }
    }
    break;
    case 2: {
      Buffer<uint16_t> buffer(fp, buffer_size, m_nz, total_size);
      for (int t=0; t<m_ntrace; ++t) {
        int i = clamp((t % m_nx) / fw, 0, m_width - 1);
        int j = clamp((t / m_nx) / fh, 0, m_height - 1);

        uint16_t* trace = buffer.NextBlock();
        if (trace == NULL) {
          free(normalized_values);
          free(n_values);
          return false;
        }

        for (int k=0; k<m_nz; ++k) {
          int kk = clamp((m_nz - 1 - k) / fd, 0, m_depth - 1);
          int index = clamp(m_width * (kk*m_height + j) + i, 0, tex_size - 1);
          if (n_values[index] < 255) { // we use an unsigned char to store this number (it will probably not exceed this limit)
            normalized_values[index] += float(trace[k] - m_vmin.s) / (m_vmax.s - m_vmin.s);
            n_values[index]++;
          }
        }
      }
    }
    break;
    case 3: {
      Buffer<uint32_t> buffer(fp, buffer_size, m_nz, total_size);
      for (int t=0; t<m_ntrace; ++t) {
        int i = clamp((t % m_nx) / fw, 0, m_width - 1);
        int j = clamp((t / m_nx) / fh, 0, m_height - 1);

        uint32_t* trace = buffer.NextBlock();
        if (trace == NULL) {
          free(normalized_values);
          free(n_values);
          return false;
        }

        for (int k = 0; k < m_nz; ++k) {
          int kk = clamp((m_nz - 1 - k) / fd, 0, m_depth - 1);
          int index = clamp(m_width * (kk*m_height + j) + i, 0, tex_size - 1);
          if (n_values[index] < 255) { // we use an unsigned char to store this number (it will probably not exceed this limit)
            normalized_values[index] += float(trace[k] - m_vmin.i) / (m_vmax.i - m_vmin.i);
            n_values[index]++;
          }
        }
      }
    }
    break;
    case 4: {
      Buffer<float> buffer(fp, buffer_size, m_nz, total_size);
      for (int t=0; t<m_ntrace; ++t) {
        int i = clamp((t % m_nx) / fw,0,m_width-1);
        int j = clamp((t / m_nx) / fh,0,m_height-1);

        float* trace = buffer.NextBlock();
        if (trace == NULL) {
          free(normalized_values);
          free(n_values);
          return false;
        }

        for (int k=0; k<m_nz; ++k) {
          int kk = clamp((m_nz - 1 - k) / fd, 0, m_depth - 1);
          int index = clamp(m_width * (kk*m_height + j) + i,0,tex_size-1);
          if (n_values[index] < 255) { // we use an unsigned char to store this number (it will probably not exceed this limit)
            normalized_values[index] += (trace[k] - m_vmin.f) / (m_vmax.f - m_vmin.f);
            n_values[index]++;
          }
        }
      }
    }
    break;
  }
  m_tex = (unsigned char*) calloc(tex_size, sizeof(unsigned char));
  if (m_tex == NULL) {
    free(normalized_values);
    free(n_values);
    return false;
  }

  for (int i=0; i<tex_size; ++i) {
    if (n_values[i]) {
      float normalized_val = normalized_values[i] / n_values[i];
      normalized_val = clamp(normalized_val, 0.0f, 1.0f);
      m_tex[i] = StoredValue(normalized_val);
    }
  }

  free(normalized_values);
  free(n_values);
  
  printf("3D texture size:\nwidth(fw) height(fh) depth(fd): %d(%d)  %d(%d)  %d(%d)\n",m_width,fw,m_height,fh,m_depth,fd);
  return true;
}

int ResSeismicData::SizeOfValue () const
{
  switch(m_vtype) {
    case 1: return 1;
    case 2: return 2;
    case 3: return 4;
    case 4: return 4;
  }
  return 0;
}

void ResSeismicData::GetBoxDeltas (glm::vec3& dx, glm::vec3& dy, glm::vec3& dz) const
{
	glm::vec3 axis = GetBoxAxis();
	glm::vec3 bu = GetBoxDirX();
	glm::vec3 bv = GetBoxDirY();
	glm::vec3 bw = GetBoxDirZ();
  dx = glm::vec3(     axis.x,       bu.y*axis.x/bu.x,  bu.z*axis.x/bu.x);
  dy = glm::vec3(bv.x*axis.y/bv.y,       axis.y,       bv.z*axis.y/bv.y);
  dz = glm::vec3(bw.x*axis.z/bw.z,  bw.y*axis.z/bw.z,       axis.z     );
}


/****************************************************************
 * Buffer Methods
 ****************************************************************/

template<class T>
ResSeismicData::Buffer<T>::Buffer (FILE* file, size_t size, size_t block, size_t total)
: m_file(file)
, m_buffer(NULL)
, m_size(size)
, m_block(block)
, m_pending(total)
, m_total_size(total)
, m_read_pos(size)
{
  m_buffer = (T*) malloc(m_size * sizeof(T));
}

template<class T>
ResSeismicData::Buffer<T>::~Buffer ()
{
  free(m_buffer);
}

template<class T>
T* ResSeismicData::Buffer<T>::NextBlock ()
{
  if (m_size - m_read_pos < m_block) {
    for (unsigned int i = 0; i < m_size - m_read_pos; i++)
      m_buffer[i] = m_buffer[m_read_pos+i];

    size_t to_read = (size_t) fmin(m_read_pos, m_pending);
    if (fread(&m_buffer[m_size - m_read_pos], sizeof(T), to_read, m_file) != to_read)
      return NULL;

    m_pending -= to_read;
    m_read_pos = 0;
  }

  size_t block_pos = m_read_pos;
  m_read_pos += m_block;

  return &m_buffer[block_pos];
}
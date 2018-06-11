#version 400

in vec3 vert_eye;
in vec3 vert_dir;

layout (location = 0) out vec4 FragColor;

uniform mat4 ViewMatrix;
uniform sampler3D BricksBuffer;
uniform sampler1D TexTransferFunc;  
uniform vec3 VolumeDimension;
uniform vec3 BrickDimension;
uniform float StepSize;
uniform vec3 BricksCoords[585];

struct Ray {
  vec3 Origin;
  vec3 Dir;
};

struct AABB {
  vec3 Min;
  vec3 Max;
};

ivec3 fWorld(vec3 worldPosition)
{
  int i = int(floor(worldPosition.x / BrickDimension.x));
  int j = int(floor(worldPosition.y / BrickDimension.x));
  int k = int(floor(worldPosition.z / BrickDimension.x));
  return ivec3(i,j,k);
}

int fid(int level, ivec3 ijk)
{
  int N  = int(floor((pow(8,level) - 1) / 7));
  int nn = int(pow(2, level));
  int t = N + (ijk.x*nn + ijk.y)*nn + ijk.z;
  return t;
}

ivec3 fijk(int level, int id)
{
  int n = int(pow(2,level));
  int n2 = int(pow(n,2));
  int delta = id - int(floor((pow(8,level) - 1) / 7));
  int i = int(floor(delta/n2));
  int j = int(mod(floor(delta/n),n));
  int k = int(mod(mod(delta,n),n));
  return ivec3(i,j,k);
}

int fparent(int level, int id)
{
  ivec3 ijk = fijk(level, id);
  int i = int(floor((ijk.x - 1)/2));
  int j = int(floor((ijk.y - 1)/2));
  int k = int(floor((ijk.z - 1)/2));
  int p = fid((level-1), ivec3(i,j,k));
  return p;
}

vec4 getTexelTest(vec3 in_pos)
{
  float xNormalized = (in_pos.x / 256.0);
  float yNormalized = (in_pos.y / 256.0);
  float zNormalized = (in_pos.z / 256.0);
  //if(xNormalized <= 1 && yNormalized <= 1 && zNormalized <= 1)
  //  return vec4(0.2,1.0,0.0,0.2);
  vec3 coords = vec3(xNormalized, yNormalized, zNormalized);
  float vol_density = texture(BricksBuffer, coords).r;
  return texture(TexTransferFunc, vol_density);
}

vec4 getTexel(vec3 in_pos)
{
  // bottom-up, é o menor nível !
  int level = 3;  
  ivec3 ijk = fWorld(in_pos);
  // ID do tile de menor tamanho que contem o ponto ;
  int id = fid(level, ijk);
  vec3 positionStorage = BricksCoords[id];

  // isso ocorre somente no máximo a altura da árvore
  // ou seja, o numero de levels
  while(level >= 0)
  {
    positionStorage = BricksCoords[id];
    if(positionStorage.x == -1)
    {
      id = fparent(level, id);
      level = level -1;
    }
    else
    {
      break;
    }
  }

  int dimension = int(256 / pow(2,level));

  ivec3 tileIndex = fijk(level, id);

  vec3 min = tileIndex * dimension;
  vec3 max = min + vec3(dimension,dimension,dimension);
  float xn = (in_pos.x - min.x)/(max.x - min.x);
  float yn = (in_pos.y - min.y)/(max.y - min.y);
  float zn = (in_pos.z - min.z)/(max.z - min.z);
  float xNormalized = (positionStorage.x / 256.0) +  (xn / 8.0) ;
  float yNormalized = (positionStorage.y / 256.0) +  (yn / 8.0) ;
  float zNormalized = (positionStorage.z / 256.0) +  (zn / 8.0) ;
  vec3 coords = vec3(xNormalized, yNormalized, zNormalized);
  float vol_density = texture(BricksBuffer, coords).r;
  return texture(TexTransferFunc, vol_density);
}

// Intersect ray with a box
// http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm
bool IntersectBox (Ray r, AABB box, out float ts, out float tf)
{
  vec3 invR = vec3(1.0) / r.Dir;
  
  vec3 tMin = invR * (box.Min - r.Origin);
  vec3 tMax = invR * (box.Max - r.Origin);
  
  vec3 t1 = min(tMin, tMax);
  vec3 t2 = max(tMin, tMax);
  
  float tnear = max(max(t1.x, t1.y), t1.z);
  float tfar  = min(min(t2.x, t2.y), t2.z);
  
  ts = tnear;
  tf = tfar;

  return tfar > tnear;
}

bool GetIntersectRayBox (out Ray r, out float s1, out float rtnear, out float rtfar)
{
  r.Origin = vert_eye;
  r.Dir = normalize(vert_dir);

  AABB aabb_box;
  aabb_box.Min = + VolumeDimension * 0.5;
  aabb_box.Max = - VolumeDimension * 0.5;

  // Find intersection with box
  float tnear, tfar;
  bool hit = IntersectBox(r, aabb_box, tnear, tfar);

  if (hit)
  {
    if (tnear < 0.0) tnear = 0.0;
    
    s1 = abs(tfar - tnear) / 1.0;
    
    rtnear = tnear;
    rtfar  = tfar;
  }

  return hit;
}

void main(void)
{
  float s1 = 1.0;

  Ray r; float tnear, tfar;
  bool inbox = GetIntersectRayBox(r, s1, tnear, tfar);

  if (!inbox) discard;

  FragColor = vec4(1.0, 1.0, 1.0, 1.0);

  vec4 dst = vec4(0);
  
  vec3 pos = r.Origin + r.Dir * tnear;

  float h = StepSize;
  float transparencyAcum = 1.0;

  float s = 0.0;
  while (s < s1)
  {
    float d = min(h, s1 - s);
    
    // Position in the real world
    pos = pos + r.Dir * d;
    
    vec3 pos_from_zero = (pos + (VolumeDimension / 2.0));
    vec4 src = getTexel(pos_from_zero);
    
    if (src.a > 0.0)
    {
      vec3 amb_color = src.rgb;
      src.rgb = amb_color;
    }

    dst.r += src.r*src.a*d * transparencyAcum;
    dst.g += src.g*src.a*d * transparencyAcum;
    dst.b += src.b*src.a*d * transparencyAcum;
    dst.a +=       src.a*d * transparencyAcum;
    transparencyAcum *= (1.0 - src.a*d);

    if(dst.a >= .95)
      break; 
  
    s = s + d;
  }

  FragColor = dst;
}
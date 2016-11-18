#include <vector>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
using namespace glm;

// The parameter h gives the fractal different shapes.
// h = 0 -> cube
// h = 0.618033989 -> dodecahedron
// h = 1 -> rhombic dodecahedron (use 0.999 for better normals)
// h = -0.5 -> endo-dodecahedron (just looks cool)
static const float h = 0.f;
static const vec3 DOD_VERTICES[20] =
{
	vec3(1.0f, 1.0f, 1.0f),
	vec3(1.0f, 1.0f, -1.0f),
	vec3(1.0f, -1.0f, 1.0f),
	vec3(1.0f, -1.0f, -1.0f),
	vec3(-1.0f, 1.0f, 1.0f),
	vec3(-1.0f, 1.0f, -1.0f),
	vec3(-1.0f, -1.0f, 1.0f),
	vec3(-1.0f, -1.0f, -1.0f),
	vec3(0.0f, 1 - h*h, 1 + h),
	vec3(0.0f, 1 - h*h, -1 - h),
	vec3(0.0f, h*h - 1, 1 + h),
	vec3(0.0f, h*h - 1, -1 - h),
	vec3(1 - h*h, 1 + h, 0.0f),
	vec3(1 - h*h, -1 - h, 0.0f),
	vec3(h*h - 1, 1 + h, 0.0f),
	vec3(h*h - 1, -1 - h, 0.0f),
	vec3(1 + h, 0.0f, 1 - h*h),
	vec3(1 + h, 0.0f, h*h - 1),
	vec3(-1 - h, 0.0f, 1 - h*h),
	vec3(-1 - h, 0.0f, h*h - 1)
};

static const vec3 DOD_NORMALS[12] =
{
	normalize(vec3(-2*(h*h*h + h*h + h), 0.0f, -2*(h*h*h + h*h - h - 1))),
	vec3(0.0f, 1.0f, 0.0f),
	vec3(0.0f, 1.0f, 0.0f),
	vec3(0.0f, 1.0f, 0.0f),
	vec3(0.0f, 1.0f, 0.0f),
	vec3(0.0f, 1.0f, 0.0f),
	vec3(0.0f, 1.0f, 0.0f),
	vec3(0.0f, 1.0f, 0.0f),
	vec3(0.0f, 1.0f, 0.0f),
	vec3(0.0f, 1.0f, 0.0f),
	vec3(0.0f, 1.0f, 0.0f),
	vec3(0.0f, 1.0f, 0.0f)
};

static const unsigned DOD_INDICES[108] =
{
	19, 5,  9,  19, 9,  11, 7,  19, 11,
	11, 9,  17, 17, 9,  1,  3,  11, 17,
	12, 17, 1,  16, 17, 12, 12, 0,  16,
	16, 13, 17, 13, 3,  17, 2,  13, 16,
	14, 12, 9,  9,  5,  14, 12, 1,  9,
	12, 14, 8,  8,  0,  12, 14, 4,  8,
	18, 10, 8,  6,  10, 18, 8,  4,  18,
	16, 8,  10, 0,  8,  16, 10, 2,  16,
	14, 19, 18, 5,  19, 14, 4,  14, 18,
	18, 19, 15, 6,  18, 15, 19, 7,  15,
	15, 11, 13, 11, 3,  13, 15, 7,  11,
	13, 10, 15, 15, 10, 6,  13, 2,  10
};

// This method generates a fractal dodecahedron
// _levels: number of subdivisions
// _vertexCount: the number of generated vertices. There are 27/5 as many indices.
//		This must be 0 on first call.
void genFractal(int _levels, float _scale, vec3 _translation, std::vector<vec3>& _positions, std::vector<vec3>& _normals, std::vector<unsigned>& _indices)
{
	if(_levels == 0)
	{
		unsigned vLUT[9];
		// Create a scaled and translated dodecahedron
		// Iterate per face.
		for(int i = 0; i < 12; ++i)
		{
			vec3 normal = normalize(cross(DOD_VERTICES[DOD_INDICES[i*9 + 1]] - DOD_VERTICES[DOD_INDICES[i*9]],
										  DOD_VERTICES[DOD_INDICES[i*9 + 2]] - DOD_VERTICES[DOD_INDICES[i*9]]));
			for(int j = 0; j < 9; ++j)
			{
				vLUT[j] = -1;
				// The indices are sorted per face -> there are 5 unique
				// vertices which must be added.
				for(int k = 0; k < j; ++k) {
					if(DOD_INDICES[i*9 + k] == DOD_INDICES[i*9 + j]) {
						vLUT[j] = vLUT[k];
						break;
					}
				}
				if(vLUT[j] == -1)
				{
					vLUT[j] = static_cast<unsigned>(_positions.size());
					_positions.push_back( DOD_VERTICES[DOD_INDICES[i*9+j]] * _scale + _translation );
					//_normals.push_back( DOD_NORMALS[i] );
					_normals.push_back( normal );
				}
				_indices.push_back( vLUT[j] );
			}
		}
	} else {
		float subScale = _scale * 0.34f;
		for(int i = 0; i < 20; ++i)
		{
			// Move the octahedron, such that the downscaled vertex i matches the original i.
			vec3 translation = DOD_VERTICES[i] * _scale + _translation - DOD_VERTICES[i] * subScale;
			genFractal(_levels - 1, subScale, translation, _positions, _normals, _indices);
		}
	}
}
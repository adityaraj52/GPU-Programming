#include "objloader.hpp"

#include <iostream>
#include <unordered_map>
#include <cassert>
#include <string>
#include <glm/gtx/orthonormalize.hpp>

using namespace glm;

static float saturate(float x) { return x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x; }
struct ivec3hash {
public:
	std::size_t operator()(const ivec3& x) const
	{
		return std::hash<int>()(x.x) * 11 + std::hash<int>()(x.y) * 3 + std::hash<int>()(x.z);
	}
};

/*!max:re2c*/
/*!re2c re2c:define:YYCTYPE = "unsigned char"; */

static float parseFloat(const unsigned char *& _string)
{
	double d = 0;
	double x = 1;
	float s = 1;
	int e = 0;
	/*!re2c
		re2c:yyfill:enable = 0;
		re2c:define:YYCURSOR = _string;
	*/
mant_int:
    /*!re2c
        "."   { goto mant_frac; }
        [eE]  { goto exp_sign; }
        [0-9] { d = (d * 10) + (_string[-1] - '0'); goto mant_int; }
		[ \t] { goto mant_int; }
		"-"   { s = -1.0f; goto mant_int; }
		*     { goto end; }
    */
mant_frac:
    /*!re2c
        *     { goto sfx; }
        [eE]  { goto exp_sign; }
        [0-9] { d += (x /= 10) * (_string[-1] - '0'); goto mant_frac; }
    */
exp_sign:
    /*!re2c
        "+"?  { x = 1e+1; goto exp; }
        "-"   { x = 1e-1; goto exp; }
    */
exp:
    /*!re2c
        *     { for (; e > 0; --e) d *= x;    goto sfx; }
        [0-9] { e = (e * 10) + (_string[-1] - '0'); goto exp; }
    */
sfx:
    /*!re2c
        *     { goto end; }
        [fF]  { if (d > FLT_MAX) d = FLT_MAX; goto end; }
    */
end:
	return s * static_cast<float>(d);
}

static vec3 parseVector(const unsigned char * _string)
{
	vec3 vertex;
	vertex.x = parseFloat(_string); _string--;
	vertex.y = parseFloat(_string); _string--;
	vertex.z = parseFloat(_string);
	return vertex;
}

static void parseIndices(const unsigned char * _string, ivec3& _vertexIndex, ivec3& _texCoordIndex, ivec3& _normalIndex)
{
	int i = 0;
	int indexType = 0;
	ivec3* indexSets[3] = {&_vertexIndex, &_texCoordIndex, &_normalIndex};
	/*!re2c
		re2c:yyfill:enable = 0;
		re2c:define:YYCURSOR = _string;
	*/
	unsigned num = 0;
	while(true) {
		if(i >= 3) break;
	/*!re2c
		""    { if(num != 0) (*indexSets[indexType])[i] = num; break; }
		[0-9] { num = num * 10 + (_string[-1] - '0'); continue; }
		"/"   {
			if(num != 0)
				(*indexSets[indexType])[i] = num;
			indexType = (indexType + 1) % 3;
			num = 0;
			continue;
		}
		* {
			if(num != 0)
			{
				(*indexSets[indexType])[i] = num;
				++i;
				indexType = 0;
				num = 0;
			}
			continue;
		}
	*/
	}
}

// The loader implementation follows:
// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
void gpupro::OBJLoader::load(const char* _fileName, bool _computeTangentSpace)
{
	m_positions.clear();
	m_tangentSpaces.clear();
	m_texCoords.clear();
	m_indices.clear();

	FILE* file = fopen(_fileName, "rb");
	struct FileCloseGuard {FILE* file; FileCloseGuard(FILE* _file) : file(_file) {} ~FileCloseGuard() { fclose(file); } } guard(file);
	if(!file) {
		std::cerr << "ERR: Cannot open file: " << _fileName << '\n';
		return;
	}

	// Temporary buffers for all data
	std::vector<vec3> vertices;
	std::vector<vec3> normals;
	std::vector<vec2> texCoords;
	std::vector<ivec3> vertexIndices;
	std::vector<ivec3> normalIndices;
	std::vector<ivec3> texCoordIndices;

	// Read line by line
	while(true)
	{
		// Get line
		char buf[128];
		if(!fgets(buf, 128, file))
			break;

		const unsigned char * cursor = (unsigned char*)buf;
		/*!re2c
			re2c:yyfill:enable = 0;
			re2c:define:YYCURSOR = cursor;

			*		{ continue; }
			'vn'	{ normals.push_back(parseVector(cursor)); continue; }
			'vt'	{ texCoords.push_back(vec2(parseVector(cursor))); continue; }
			'v'		{ vertices.push_back(parseVector(cursor)); continue; }
			'f'		{
				// Obj is 1-indexed. Use this as fall back if parsing fails for some data.
				ivec3 vertexIndex(1), normalIndex(1), texCoordIndex(1);
				
				parseIndices(cursor, vertexIndex, texCoordIndex, normalIndex);
				
				vertexIndices.push_back(vertexIndex);
				texCoordIndices.push_back(texCoordIndex);
				normalIndices.push_back(normalIndex);
				continue;
			}
		*/
	}

	// Since texture coordinates and normals are optional add one dummy coordinate.
	texCoords.push_back(vec2(0.0f));
	normals.push_back(vec3(0.0f, 1.0f, 0.0f));

	// Now there are three index buffers. Since OpenGL can handle only one the data
	// must be transformed.
	unsigned index = 0;
	std::unordered_map<ivec3, unsigned, ivec3hash> indexMap;
	TangentSpace dummySpace;
	dummySpace.tangent = vec3(0.0f);
	dummySpace.bitangent = vec3(0.0f);

	for(size_t i = 0; i < vertexIndices.size(); ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			ivec3 indexTriple( vertexIndices[i][j], normalIndices[i][j], texCoordIndices[i][j] );
			auto it = indexMap.find(indexTriple);
			if( it != indexMap.end() )
			{
				m_indices.push_back(it->second);
			} else
			{
				m_positions.push_back(vertices[indexTriple.x-1]);
				dummySpace.normal = normals[indexTriple.y-1];
				m_tangentSpaces.push_back(dummySpace);
				m_texCoords.push_back(texCoords[indexTriple.z-1]);
				m_indices.push_back(index);
				indexMap[indexTriple] = index;
				index++;
			}
		}
	}

	if(_computeTangentSpace)
		computeTangentSpace();
}

void gpupro::OBJLoader::computeTangentSpace()
{
	// Get tangent spaces on triangles and average them on vertex locations
	for(size_t i = 0; i < m_indices.size(); i += 3)
	{
		vec3 e0 = m_positions[m_indices[i+1]] - m_positions[m_indices[i]];
		vec3 e1 = m_positions[m_indices[i+2]] - m_positions[m_indices[i]];
		vec3 e2 = m_positions[m_indices[i+2]] - m_positions[m_indices[i+1]];
		vec3 triNormal, triTangent, triBitangent;
		triNormal = normalize(cross(e0, e1));
		assert((triNormal == triNormal) && "NaN in normal computation!");

		vec2 uva = m_texCoords[m_indices[i+1]] - m_texCoords[m_indices[i]];
		vec2 uvb = m_texCoords[m_indices[i+2]] - m_texCoords[m_indices[i]];
		float det = uva.x * uvb.y - uva.y * uvb.x; // may swap the sign
		if(det == 0.0f) det = 1.0f;
		triTangent = (uvb.y * e0 - uva.y * e1) / det;
		triBitangent = (uva.x * e1 - uvb.x * e0) / det;
		// Try to recover direction if it got NaN
		if(length(triTangent) < 1e-3f && length(triBitangent) < 1e-3f)
		{
			// Create a random orthonormal basis (no uv given)
			triTangent = vec3(1.0f, triNormal.x, 0.0f);
			triBitangent = vec3(0.0f, triNormal.z, 1.0f);
		} else if(!(triTangent == triTangent) || length(triTangent) < 1e-3f)
			triTangent = cross(triBitangent, triNormal) * det;
		else if(!(triBitangent == triBitangent) || length(triBitangent) < 1e-3f)
			triBitangent = cross(triNormal, triTangent) * det;
		triTangent = orthonormalize(triTangent, triNormal);
		triBitangent = orthonormalize(triBitangent, triNormal);
		assert((triTangent == triTangent) && "NaN in tangent computation!");
		assert((triBitangent == triBitangent) && "NaN in bitangent computation!");
		assert((abs(length(triTangent) - 1.0f) < 1e-4f) && "Computed tangent has a wrong length!");
		assert((abs(length(triBitangent) - 1.0f) < 1e-4f) && "Computed bitangent has a wrong length!");

		// Add to all adjacent vertices
		float lenE0 = length(e0), lenE1 = length(e1), lenE2 = length(e2);
		float weight = acos(saturate(dot(e0, e1) / (lenE0 * lenE1)));
		m_tangentSpaces[m_indices[i]].tangent += triTangent * weight;
		m_tangentSpaces[m_indices[i]].bitangent += triBitangent * weight;
		weight = acos(saturate(-dot(e0, e2) / (lenE0 * lenE2)));
		m_tangentSpaces[m_indices[i+1]].tangent += triTangent * weight;
		m_tangentSpaces[m_indices[i+1]].bitangent += triBitangent * weight;
		weight = acos(saturate(dot(e1, e2) / (lenE1 * lenE2)));
		m_tangentSpaces[m_indices[i+2]].tangent += triTangent * weight;
		m_tangentSpaces[m_indices[i+2]].bitangent += triBitangent * weight;
	}

	// Orthonormalize
	for(size_t i = 0; i < m_positions.size(); ++i)
	{
		m_tangentSpaces[i].tangent = orthonormalize(m_tangentSpaces[i].tangent, m_tangentSpaces[i].normal);
		m_tangentSpaces[i].bitangent = orthonormalize(m_tangentSpaces[i].bitangent, m_tangentSpaces[i].normal);
	}
}

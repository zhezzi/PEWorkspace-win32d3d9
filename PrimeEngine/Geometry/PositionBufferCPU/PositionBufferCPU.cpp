#define NOMINMAX

// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Immediate include
#include "PositionBufferCPU.h"

// Inter-Engine includes
#include "PrimeEngine/FileSystem/FileReader.h"
#include "PrimeEngine/Utils/StringOps.h"
#include "PrimeEngine/MainFunction/MainFunctionArgs.h"

#include "PrimeEngine/Math/Vector3.h" 

// Sibling/Children includes

// Reads the specified buffer from file
void PositionBufferCPU::ReadPositionBuffer(const char *filename, const char *package)
{
    PEString::generatePathname(*m_pContext, filename, package, "PositionBuffers", PEString::s_buf, PEString::BUF_SIZE);

	// Path is now a full path to the file with the filename itself
	FileReader f(PEString::s_buf);

	char line[256];
	f.nextNonEmptyLine(line, 255);
	// TODO : make sure it is "POSITION_BUFFER"
	int version = 0;
	if (0 == StringOps::strcmp(line, "POSITION_BUFFER_V1"))
	{
		version = 1;
	}

	PrimitiveTypes::Int32 n;
	f.nextInt32(n);
	m_values.reset(n * 3); // 3 Float32 per vertex

	float factor = version == 0 ? (1.0f / 100.0f) : 1.0f;

	// Read all values
	PrimitiveTypes::Float32 val;
	for (int i = 0; i < n * 3; i++)
	{
		f.nextFloat32(val);
		m_values.add(val * factor);
	}

	// Initialize the maximum values with small values and minimum values with large values
	Vector3 max_vertex(std::numeric_limits<float>::lowest(),
		std::numeric_limits<float>::lowest(),
		std::numeric_limits<float>::lowest());

	Vector3 min_vertex(std::numeric_limits<float>::max(),
		std::numeric_limits<float>::max(),
		std::numeric_limits<float>::max());

	// Iterate through all vertex and find maximum values and minimum values
	for (size_t i = 0; i < m_values.m_size; i += 3) {
		float x = m_values[i];
		float y = m_values[i + 1];
		float z = m_values[i + 2];

		// Update min_vertex
		if (x < min_vertex.m_x) min_vertex.m_x = x;
		if (y < min_vertex.m_y) min_vertex.m_y = y;
		if (z < min_vertex.m_z) min_vertex.m_z = z;

		// Update max_vertex
		if (x > max_vertex.m_x) max_vertex.m_x = x;
		if (y > max_vertex.m_y) max_vertex.m_y = y;
		if (z > max_vertex.m_z) max_vertex.m_z = z;
	}

	// Printout min_vertex value and max_vertex value
	PEINFO("......Location:PositionBufferCPU: Min X: %f, Min Y: %f, Min Z: %f\n", min_vertex.m_x, min_vertex.m_y, min_vertex.m_z);
	PEINFO("......Location:PositionBufferCPU: Max X: %f, Max Y: %f, Max Z: %f\n", max_vertex.m_x, max_vertex.m_y, max_vertex.m_z);

	this->min_vertex = min_vertex;
	this->max_vertex = max_vertex;



}

void PositionBufferCPU::createEmptyCPUBuffer()
{
	m_values.reset(0);
}

void PositionBufferCPU::createBillboardCPUBuffer(PrimitiveTypes::Float32 w, PrimitiveTypes::Float32 h)
{
	m_values.reset(3 * 4);
	add3Floats(-w/2, 0.0f, -h/2);
	add3Floats(-w/2, 0.0f, h/2); 
	add3Floats(w/2, 0.0f, h/2);
	add3Floats(w/2, 0.0f, -h/2);
}
void PositionBufferCPU::createNormalizeBillboardCPUBufferXYWithPtOffsets(PrimitiveTypes::Float32 dx, PrimitiveTypes::Float32 dy)
{
	m_values.reset(3 * 4);
	m_values.add(-1.0f+dx); m_values.add(-1.0f+dy); m_values.add(0.0f);
	m_values.add(1.0f+dx); m_values.add(-1.0f+dy); m_values.add(0.0f);
	m_values.add(1.0f+dx); m_values.add(1.0f+dy); m_values.add(0.0f);
	m_values.add(-1.0f+dx); m_values.add(1.0f+dy); m_values.add(0.0f);
	
}
#include "ecs/cache/ComponentsTuple.hpp"

namespace ecs
{

ComponentsTuple::ComponentsTuple(const std::size_t tupleSize)
	: m_size(tupleSize)
{
	if (m_size > 0)
	{
		m_data = new ComponentPtr[m_size];
	}
}

ComponentsTuple::~ComponentsTuple()
{
	if (nullptr != m_data)
	{
		delete[] m_data;
		m_data = nullptr;
	}
}

// Move is enabled
ComponentsTuple::ComponentsTuple(ComponentsTuple&& other)
	: m_data(other.m_data)
	, m_size(other.m_size)
{
	other.m_data = nullptr;
	other.m_size = 0;
}

ComponentsTuple& ComponentsTuple::operator=(ComponentsTuple&& other)
{
	m_data = other.m_data;
	m_size = other.m_size;

	other.m_data = nullptr;
	other.m_size = 0;

	return *this;
}

const ComponentPtr* ComponentsTuple::GetData() const
{
	return m_data;
}

ComponentPtr* ComponentsTuple::GetMutableData()
{
	return m_data;
}

const std::size_t ComponentsTuple::GetSize() const
{
	return m_size;
}

ComponentPtr& ComponentsTuple::operator[](const std::size_t index)
{
	return m_data[index];
}

const ComponentPtr& ComponentsTuple::operator[](const std::size_t index) const
{
	return m_data[index];
}

}

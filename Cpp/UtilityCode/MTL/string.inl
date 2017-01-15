//
// Mini Template Library
//

#ifndef MTL_namespace
#error MTL_namespace must be defined
#endif

#ifndef MTL_assert
#error MTL_assert must be defined
#endif

#ifndef MTL_memcpy
#error MTL_memcpy must be defined
#endif

#ifndef MTL_memset
#error MTL_memset must be defined
#endif

#ifndef MTL_strlen
#error MTL_strlen must be defined
#endif

#ifndef MTL_new
#error MTL_new must be defined
#endif

#ifndef MTL_new_array
#error MTL_new_array must be defined
#endif

#ifndef MTL_delete
#error MTL_delete must be defined
#endif

#ifndef MTL_delete_array
#error MTL_delete_array must be defined
#endif

namespace MTL_namespace {
///////////////////////////////////////////////////////////////////////////////
// Very simple string implementation
class string_view
{
public:
	string_view()
		: m_Data(nullptr), m_Size(0)
	{
	}

	string_view(const char* data, size_t size)
		: m_Data(data), m_Size(size)
	{
	}

	string_view(const char* data)
		: string_view(data, MTL_strlen(data))
	{
	}

	const char* data() const
	{
		return m_Data;
	}

	size_t size() const
	{
		return m_Size;
	}

private:
	const char* m_Data;
	size_t m_Size;
};

class string
{
public:
	string()
	{
	}

	string(const char* data, size_t size)
	{
		Assign(data, size);
	}

	string(const char* data)
	{
		Assign(data, MTL_strlen(data));
	}

	string(const string_view& sv)
	{
		Assign(sv.data(), sv.size());
	}

	string(string&& str)
		: m_Data(str.m_Data), m_Size(str.m_Size), 
		m_Capacity(str.m_Capacity)
	{
		str.Detach();
	}

	string(size_t size, char ch)
	{
		Resize(size, true, ch);
	}

	~string()
	{
		ReleaseBuffer();
	}

	const char* c_str() const
	{
		return m_Data;
	}

	const char* data() const
	{
		return m_Data;
	}

	char* data()
	{
		return m_Data;
	}

	size_t size() const
	{
		return m_Size;
	}

	operator string_view() const
	{
		return string_view(m_Data, m_Size);
	}

	void resize(size_t newSize)
	{
		Resize(newSize);
	}

	void resize(size_t newSize, char ch)
	{
		Resize(newSize, true, ch);
	}

	void clear()
	{
		Resize(0);
	}

private:
	string_view Detach()
	{
		char* data = m_Data;
		size_t size = m_Size;
		m_Data = nullptr;
		m_Size = 0;
		m_Capacity = 0;
		return string_view(data, size);
	}

	void Resize(size_t newSize, bool fill = false, char ch = 0)
	{
		if (!m_Data)
		{
			MTL_assert(!m_Size && !m_Capacity);

			m_Size = newSize;
			m_Capacity = newSize;
			m_Data = MTL_new_array(char, newSize + 1);
			m_Data[newSize] = 0;
			if (fill)
			{
				MTL_memset(m_Data, ch, m_Size);
			}
			return;
		}
		
		if (newSize <= m_Capacity)
		{
			m_Size = newSize;
			m_Data[newSize] = 0;
			return;
		}

		string_view old = Detach();
		const size_t oldSize = old.size();
		const char* oldData = old.data();

		m_Size = newSize;
		m_Capacity = newSize;
		m_Data = MTL_new_array(char, newSize + 1);
		m_Data[newSize] = 0;

		if (newSize > oldSize)
		{
			const size_t diff = newSize - oldSize;
			MTL_memcpy(m_Data, oldData, oldSize);
			if (fill)
			{
				MTL_memset(&m_Data[oldSize], ch, diff);
			}
		}
		else
		{
			MTL_assert(0);
		}

		MTL_delete_array(const_cast<char*>(oldData));
	}

	void ReleaseBuffer()
	{
		if (m_Data)
		{
			MTL_assert(m_Size && m_Capacity);
			MTL_delete_array(m_Data);
		}
		m_Data = nullptr;
		m_Size = 0;
		m_Capacity = 0;
	}

	void Assign(const char* data, size_t size)
	{
		Resize(size);
		MTL_memcpy(m_Data, data, m_Size);
	}

private:
	char* m_Data = nullptr;
	size_t m_Size = 0;
	size_t m_Capacity = 0;
};

inline string operator + (const string& lhs, const string& rhs)
{
	const auto lhsSize = lhs.size();
	const auto rhsSize = rhs.size();

	string s(lhsSize + rhsSize, 0);
	char* p = s.data();
	MTL_memcpy(p, lhs.data(), lhsSize);
	MTL_memcpy(&p[lhsSize], rhs.data(), rhsSize);

	return s;
}
}

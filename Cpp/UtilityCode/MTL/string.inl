//
// Mini Template Library
//

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
		: m_Data(str.m_Data), m_Size(str.m_Size)
	{
		str.m_Data = nullptr;
		str.m_Size = 0;
	}

	string(size_t size, char ch)
	{
		AllocFilled(size, ch);
	}

	~string()
	{
		Release();
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
		Resize(newSize, 0);
	}

	void resize(size_t newSize, char ch)
	{
		Resize(newSize, ch);
	}

	void clear()
	{
		Release();
	}

private:
	string_view Detach()
	{
		char* data = m_Data;
		size_t size = m_Size;
		m_Data = nullptr;
		m_Size = 0;
		return string_view(data, size);
	}

	void Resize(size_t newSize, char ch)
	{
		if (!m_Data)
		{
			AllocFilled(newSize, ch);
			return;
		}
		
		if (newSize <= m_Size)
		{
			m_Size = newSize;
			m_Data[newSize] = 0;
			return;
		}

		string_view old = Detach();
		const size_t oldSize = old.size();
		const char* oldData = old.data();

		Alloc(newSize);
		if (newSize > oldSize)
		{
			const size_t diff = newSize - oldSize;
			MTL_memcpy(m_Data, oldData, oldSize);
			MTL_memset(&m_Data[oldSize], ch, diff);
		}

		char* p = const_cast<char*>(oldData);
		MTL_delete_array(p);
	}

	void Release()
	{
		if (m_Data)
			MTL_delete_array(m_Data);
		m_Data = nullptr;
		m_Size = 0;
	}

	void Alloc(size_t size)
	{
		if (m_Data)
			Release();
		m_Size = size;
		m_Data = MTL_new_array(char, size + 1);
		m_Data[size] = 0;
	}

	void AllocFilled(size_t size, char ch)
	{
		Alloc(size);
		MTL_memset(m_Data, ch, m_Size);
	}

	void Assign(const char* data, size_t size)
	{
		Alloc(size);
		MTL_memcpy(m_Data, data, m_Size);
	}

private:
	char* m_Data = nullptr;
	size_t m_Size = 0;
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

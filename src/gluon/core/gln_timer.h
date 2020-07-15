#include <EASTL/chrono.h>

namespace gluon
{
using clock     = eastl::chrono::high_resolution_clock;
using timepoint = eastl::chrono::high_resolution_clock::time_point;

class timer
{
public:
	inline void Start()
	{
		m_Start    = clock::now();
		m_LastTime = m_Start;
	}

	inline void Restart() { Start(); }

	inline double DeltaTime() const
	{
		const timepoint                 Time = clock::now();
		eastl::chrono::duration<double> dt   = Time - m_LastTime;

		m_LastTime = Time;

		return dt.count();
	}

	inline double GetElapsedSeconds() const
	{
		const timepoint                 Time = clock::now();
		eastl::chrono::duration<double> dt   = Time - m_Start;

		return dt.count();
	}

private:
	timepoint         m_Start;
	mutable timepoint m_LastTime;
};

}

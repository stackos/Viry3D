#pragma once

namespace Viry3D
{
	class Stream
	{
	public:
		Stream();
		virtual ~Stream() { }
		virtual void Close();
		virtual int Read(void* buffer, int size);
		virtual int Write(void* buffer, int size);

	protected:
		int m_position;
		bool m_closed;
		int m_length;
	};
}
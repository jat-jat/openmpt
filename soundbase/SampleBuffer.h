/*
 * SampleBuffer.h
 * --------------
 * Purpose: C++2b audio_buffer-like thingy.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#pragma once

#include "BuildSettings.h"

#include "../common/mptBaseMacros.h"
#include "../common/mptBaseTypes.h"


OPENMPT_NAMESPACE_BEGIN


template <typename SampleType>
struct audio_buffer_planar
{
public:
	using sample_type = SampleType;

private:
	SampleType *const *m_buffers;
	std::size_t m_channels;
	std::size_t m_frames;

public:
	constexpr audio_buffer_planar(SampleType *const *buffers, std::size_t channels, std::size_t frames) noexcept
		: m_buffers(buffers)
		, m_channels(channels)
		, m_frames(frames)
	{
		return;
	}
	SampleType *data() noexcept
	{
		return nullptr;
	}
	const SampleType *data() const noexcept
	{
		return nullptr;
	}
	SampleType &operator()(std::size_t channel, std::size_t frame)
	{
		return m_buffers[channel][frame];
	}
	const SampleType &operator()(std::size_t channel, std::size_t frame) const
	{
		return m_buffers[channel][frame];
	}
	bool is_contiguous() const noexcept
	{
		return false;
	}
	bool channels_are_contiguous() const noexcept
	{
		return false;
	}
	bool frames_are_contiguous() const noexcept
	{
		return false;
	}
	std::size_t size_channels() const noexcept
	{
		return m_channels;
	}
	std::size_t size_frames() const noexcept
	{
		return m_frames;
	}
	std::size_t size_samples() const noexcept
	{
		return m_channels * m_frames;
	}
};


template <typename SampleType>
struct audio_buffer_contiguous
{
public:
	using sample_type = SampleType;

private:
	SampleType *const m_buffer;
	std::size_t m_channels;
	std::size_t m_frames;

public:
	constexpr audio_buffer_contiguous(SampleType *buffer, std::size_t channels, std::size_t frames) noexcept
		: m_buffer(buffer)
		, m_channels(channels)
		, m_frames(frames)
	{
		return;
	}
	SampleType *data() noexcept
	{
		return m_buffer;
	}
	const SampleType *data() const noexcept
	{
		return m_buffer;
	}
	SampleType &operator()(std::size_t channel, std::size_t frame)
	{
		return m_buffer[(m_frames * channel) + frame];
	}
	const SampleType &operator()(std::size_t channel, std::size_t frame) const
	{
		return m_buffer[(m_frames * channel) + frame];
	}
	bool is_contiguous() const noexcept
	{
		return true;
	}
	bool channels_are_contiguous() const noexcept
	{
		return true;
	}
	bool frames_are_contiguous() const noexcept
	{
		return false;
	}
	std::size_t size_channels() const noexcept
	{
		return m_channels;
	}
	std::size_t size_frames() const noexcept
	{
		return m_frames;
	}
	std::size_t size_samples() const noexcept
	{
		return m_channels * m_frames;
	}
};


template <typename SampleType>
struct audio_buffer_interleaved
{
public:
	using sample_type = SampleType;

private:
	SampleType *const m_buffer;
	std::size_t m_channels;
	std::size_t m_frames;

public:
	constexpr audio_buffer_interleaved(SampleType *buffer, std::size_t channels, std::size_t frames) noexcept
		: m_buffer(buffer)
		, m_channels(channels)
		, m_frames(frames)
	{
		return;
	}
	SampleType *data() noexcept
	{
		return m_buffer;
	}
	const SampleType *data() const noexcept
	{
		return m_buffer;
	}
	SampleType &operator()(std::size_t channel, std::size_t frame)
	{
		return m_buffer[m_channels * frame + channel];
	}
	const SampleType &operator()(std::size_t channel, std::size_t frame) const
	{
		return m_buffer[m_channels * frame + channel];
	}
	bool is_contiguous() const noexcept
	{
		return true;
	}
	bool channels_are_contiguous() const noexcept
	{
		return false;
	}
	bool frames_are_contiguous() const noexcept
	{
		return true;
	}
	std::size_t size_channels() const noexcept
	{
		return m_channels;
	}
	std::size_t size_frames() const noexcept
	{
		return m_frames;
	}
	std::size_t size_samples() const noexcept
	{
		return m_channels * m_frames;
	}
};


struct audio_buffer_frames_are_contiguous_t
{
};
struct audio_buffer_channels_are_contiguous_t
{
};
struct audio_buffer_channels_are_planar_t
{
};
struct audio_buffer_channels_are_planar_and_strided_t
{
};

inline constexpr audio_buffer_frames_are_contiguous_t audio_buffer_frames_are_contiguous;
inline constexpr audio_buffer_channels_are_contiguous_t audio_buffer_channels_are_contiguous;
inline constexpr audio_buffer_channels_are_planar_t audio_buffer_channels_are_planar;
inline constexpr audio_buffer_channels_are_planar_and_strided_t audio_buffer_channels_are_planar_and_strided;

template <typename SampleType>
struct audio_buffer
{
public:
	using sample_type = SampleType;

private:
	union
	{
		SampleType *const contiguous;
		SampleType *const *const planes;
	} m_buffer;
	std::size_t m_frame_stride;
	std::size_t m_channel_stride;
	std::size_t m_channels;
	std::size_t m_frames;

public:
	constexpr audio_buffer(SampleType *buffer, std::size_t channels, std::size_t frames, audio_buffer_frames_are_contiguous_t) noexcept
		: m_frame_stride(channels)
		, m_channel_stride(1)
		, m_channels(channels)
		, m_frames(frames)
	{
		m_buffer.contiguous = buffer;
	}
	constexpr audio_buffer(SampleType *buffer, std::size_t channels, std::size_t frames, audio_buffer_channels_are_contiguous_t) noexcept
		: m_frame_stride(1)
		, m_channel_stride(frames)
		, m_channels(channels)
		, m_frames(frames)
	{
		m_buffer.contiguous = buffer;
	}
	constexpr audio_buffer(SampleType *const *planes, std::size_t channels, std::size_t frames, audio_buffer_channels_are_planar_t) noexcept
		: m_frame_stride(1)
		, m_channel_stride(0)
		, m_channels(channels)
		, m_frames(frames)
	{
		m_buffer.planes = planes;
	}
	constexpr audio_buffer(SampleType *const *planes, std::size_t channels, std::size_t frames, std::size_t frame_stride, audio_buffer_channels_are_planar_and_strided_t) noexcept
		: m_frame_stride(frame_stride)
		, m_channel_stride(0)
		, m_channels(channels)
		, m_frames(frames)
	{
		m_buffer.planes = planes;
	}
	bool is_contiguous() const noexcept
	{
		return (m_channel_stride != 0);
	}
	SampleType *data() noexcept
	{
		return is_contiguous() ? m_buffer.contiguous : nullptr;
	}
	const SampleType *data() const noexcept
	{
		return is_contiguous() ? m_buffer.contiguous : nullptr;
	}
	SampleType &operator()(std::size_t channel, std::size_t frame)
	{
		return is_contiguous() ? m_buffer.contiguous[(m_channel_stride * channel) + (m_frame_stride * frame)] : m_buffer.planes[channel][frame * m_frame_stride];
	}
	const SampleType &operator()(std::size_t channel, std::size_t frame) const
	{
		return is_contiguous() ? m_buffer.contiguous[(m_channel_stride * channel) + (m_frame_stride * frame)] : m_buffer.planes[channel][frame * m_frame_stride];
	}
	bool channels_are_contiguous() const noexcept
	{
		return (m_channel_stride == m_frames);
	}
	bool frames_are_contiguous() const noexcept
	{
		return (m_frame_stride == m_channels);
	}
	std::size_t size_channels() const noexcept
	{
		return m_channels;
	}
	std::size_t size_frames() const noexcept
	{
		return m_frames;
	}
	std::size_t size_samples() const noexcept
	{
		return m_channels * m_frames;
	}
};


template <typename Taudio_buffer>
struct audio_buffer_with_offset
{
public:
	using sample_type = typename Taudio_buffer::sample_type;

private:
	Taudio_buffer m_buffer;
	std::size_t m_offset;

public:
	audio_buffer_with_offset(Taudio_buffer buffer, std::size_t offsetFrames) noexcept
		: m_buffer(buffer)
		, m_offset(offsetFrames)
	{
		return;
	}
	sample_type *data() noexcept
	{
		if(!is_contiguous())
		{
			return nullptr;
		}
		return m_buffer.data() + (size_channels() * m_offset);
	}
	const sample_type *data() const noexcept
	{
		if(!is_contiguous())
		{
			return nullptr;
		}
		return m_buffer.data() + (size_channels() * m_offset);
	}
	sample_type &operator()(std::size_t channel, std::size_t frame)
	{
		return m_buffer(channel, m_offset + frame);
	}
	const sample_type &operator()(std::size_t channel, std::size_t frame) const
	{
		return m_buffer(channel, m_offset + frame);
	}
	bool is_contiguous() const noexcept
	{
		return m_buffer.is_contiguous() && m_buffer.frames_are_contiguous();
	}
	bool channels_are_contiguous() const noexcept
	{
		return m_buffer.channels_are_contiguous();
	}
	bool frames_are_contiguous() const noexcept
	{
		return m_buffer.frames_are_contiguous();
	}
	std::size_t size_channels() const noexcept
	{
		return m_buffer.size_channels();
	}
	std::size_t size_frames() const noexcept
	{
		return m_buffer.size_frames() - m_offset;
	}
	std::size_t size_samples() const noexcept
	{
		return size_channels() * size_frames();
	}
};


template <typename SampleType>
inline std::size_t planar_audio_buffer_valid_channels(SampleType *const *buffers, std::size_t maxChannels)
{
	std::size_t channel;
	for(channel = 0; channel < maxChannels; ++channel)
	{
		if(!buffers[channel])
		{
			break;
		}
	}
	return channel;
}


template <typename BufferType>
inline audio_buffer_with_offset<BufferType> make_audio_buffer_with_offset(BufferType buf, std::size_t numFrames) noexcept
{
	MPT_ASSERT(numFrames <= buf.size_frames());
	return audio_buffer_with_offset<BufferType>{buf, numFrames};
}


OPENMPT_NAMESPACE_END

/*
 * version.cpp
 * -----------
 * Purpose: OpenMPT version handling.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "stdafx.h"
#include "version.h"

#include "mptString.h"
#include "mptStringFormat.h"
#include "mptStringParse.h" 

#include "versionNumber.h"
#include "svn_version.h"



OPENMPT_NAMESPACE_BEGIN



static_assert((MPT_VERSION_CURRENT.GetRawVersion() & 0xffffu) != 0x0000u, "Version numbers ending in .00.00 shall never exist again, as they make interpreting the version number ambiguous for file formats which can only store the two major parts of the version number (e.g. IT and S3M).");



Version Version::Current() noexcept
{
	return MPT_VERSION_CURRENT;
}

mpt::ustring Version::GetOpenMPTVersionString() const
{
	return MPT_USTRING("OpenMPT ") + ToUString();
}

Version Version::Parse(const std::string &s)
{
	uint32 result = 0;
	std::vector<std::string> numbers = mpt::String::Split<std::string>(s, std::string("."));
	for(std::size_t i = 0; i < numbers.size() && i < 4; ++i)
	{
		result |= (mpt::String::Parse::Hex<unsigned int>(numbers[i]) & 0xff) << ((3-i)*8);
	}
	return Version(result);
}

Version Version::Parse(const mpt::ustring &s)
{
	uint32 result = 0;
	std::vector<mpt::ustring> numbers = mpt::String::Split<mpt::ustring>(s, MPT_USTRING("."));
	for (std::size_t i = 0; i < numbers.size() && i < 4; ++i)
	{
		result |= (mpt::String::Parse::Hex<unsigned int>(numbers[i]) & 0xff) << ((3 - i) * 8);
	}
	return Version(result);
}

std::string Version::ToString() const
{
	uint32 v = m_Version;
	if(v == 0)
	{
		// Unknown version
		return std::string("Unknown");
	} else if((v & 0xFFFF) == 0)
	{
		// Only parts of the version number are known (e.g. when reading the version from the IT or S3M file header)
		return mpt::format(std::string("%1.%2"))(mpt::fmt::HEX((v >> 24) & 0xFF), mpt::fmt::HEX0<2>((v >> 16) & 0xFF));
	} else
	{
		// Full version info available
		return mpt::format(std::string("%1.%2.%3.%4"))(mpt::fmt::HEX((v >> 24) & 0xFF), mpt::fmt::HEX0<2>((v >> 16) & 0xFF), mpt::fmt::HEX0<2>((v >> 8) & 0xFF), mpt::fmt::HEX0<2>((v) & 0xFF));
	}
}

mpt::ustring Version::ToUString() const
{
	uint32 v = m_Version;
	if(v == 0)
	{
		// Unknown version
		return MPT_USTRING("Unknown");
	} else if((v & 0xFFFF) == 0)
	{
		// Only parts of the version number are known (e.g. when reading the version from the IT or S3M file header)
		return mpt::format(MPT_USTRING("%1.%2"))(mpt::ufmt::HEX((v >> 24) & 0xFF), mpt::ufmt::HEX0<2>((v >> 16) & 0xFF));
	} else
	{
		// Full version info available
		return mpt::format(MPT_USTRING("%1.%2.%3.%4"))(mpt::ufmt::HEX((v >> 24) & 0xFF), mpt::ufmt::HEX0<2>((v >> 16) & 0xFF), mpt::ufmt::HEX0<2>((v >> 8) & 0xFF), mpt::ufmt::HEX0<2>((v) & 0xFF));
	}
}

Version Version::WithoutTestNumber() const noexcept
{
	return Version(m_Version & 0xFFFFFF00u);
}

Version Version::WithoutPatchOrTestNumbers() const noexcept
{
	return Version(m_Version & 0xFFFF0000u);
}

bool Version::IsTestVersion() const noexcept
{
	return (
			// Legacy
			(*this > MAKE_VERSION_NUMERIC(1,17,02,54) && *this < MAKE_VERSION_NUMERIC(1,18,02,00) && *this != MAKE_VERSION_NUMERIC(1,18,00,00))
		||
			// Test builds have non-zero VER_MINORMINOR
			(*this > MAKE_VERSION_NUMERIC(1,18,02,00) && ((m_Version & 0xFFFFFF00u) != m_Version))
		);
}



namespace Source {

static mpt::ustring GetUrl()
{
	#ifdef OPENMPT_VERSION_URL
		return mpt::ToUnicode(mpt::CharsetASCII, OPENMPT_VERSION_URL);
	#else
		return mpt::ustring();
	#endif
}

static int GetRevision()
{
	#if defined(OPENMPT_VERSION_REVISION)
		return OPENMPT_VERSION_REVISION;
	#elif defined(OPENMPT_VERSION_SVNVERSION)
		std::string svnversion = OPENMPT_VERSION_SVNVERSION;
		if(svnversion.length() == 0)
		{
			return 0;
		}
		if(svnversion.find(":") != std::string::npos)
		{
			svnversion = svnversion.substr(svnversion.find(":") + 1);
		}
		if(svnversion.find("-") != std::string::npos)
		{
			svnversion = svnversion.substr(svnversion.find("-") + 1);
		}
		if(svnversion.find("M") != std::string::npos)
		{
			svnversion = svnversion.substr(0, svnversion.find("M"));
		}
		if(svnversion.find("S") != std::string::npos)
		{
			svnversion = svnversion.substr(0, svnversion.find("S"));
		}
		if(svnversion.find("P") != std::string::npos)
		{
			svnversion = svnversion.substr(0, svnversion.find("P"));
		}
		return ConvertStrTo<int>(svnversion);
	#else
		#if MPT_COMPILER_MSVC
			#pragma message("SVN revision unknown. Please check your build system.")
		#elif MPT_COMPILER_GCC || MPT_COMPILER_CLANG
			#warning "SVN revision unknown. Please check your build system."
		#else
			// There is no portable way to display a warning.
			// Try to provoke a warning with an unused variable.
			int SVN_revision_unknown__Please_check_your_build_system;
		#endif
		return 0;
	#endif
}

static bool IsDirty()
{
	#if defined(OPENMPT_VERSION_DIRTY)
		return OPENMPT_VERSION_DIRTY != 0;
	#elif defined(OPENMPT_VERSION_SVNVERSION)
		std::string svnversion = OPENMPT_VERSION_SVNVERSION;
		if(svnversion.length() == 0)
		{
			return false;
		}
		if(svnversion.find("M") != std::string::npos)
		{
			return true;
		}
		return false;
	#else
		return false;
	#endif
}

static bool HasMixedRevisions()
{
	#if defined(OPENMPT_VERSION_MIXEDREVISIONS)
		return OPENMPT_VERSION_MIXEDREVISIONS != 0;
	#elif defined(OPENMPT_VERSION_SVNVERSION)
		std::string svnversion = OPENMPT_VERSION_SVNVERSION;
		if(svnversion.length() == 0)
		{
			return false;
		}
		if(svnversion.find(":") != std::string::npos)
		{
			return true;
		}
		if(svnversion.find("-") != std::string::npos)
		{
			return true;
		}
		if(svnversion.find("S") != std::string::npos)
		{
			return true;
		}
		if(svnversion.find("P") != std::string::npos)
		{
			return true;
		}
		return false;
	#else
		return false;
	#endif
}

static bool IsPackage()
{
	#if defined(OPENMPT_VERSION_IS_PACKAGE)
		return OPENMPT_VERSION_IS_PACKAGE != 0;
	#else
		return false;
	#endif
}

static mpt::ustring GetSourceDate()
{
	#if defined(OPENMPT_VERSION_DATE)
		return mpt::ToUnicode(mpt::CharsetASCII, OPENMPT_VERSION_DATE);
	#else
		return mpt::ustring();
	#endif
}

} // namespace Source

SourceInfo::SourceInfo()
	: m_Url(Source::GetUrl())
	, m_Revision(Source::GetRevision())
	, m_IsDirty(Source::IsDirty())
	, m_HasMixedRevisions(Source::HasMixedRevisions())
	, m_IsPackage(Source::IsPackage())
	, m_Date(Source::GetSourceDate())
{
}

mpt::ustring SourceInfo::GetUrlWithRevision() const
{
	if(m_Url.empty() || (m_Revision == 0))
	{
		return mpt::ustring();
	}
	return m_Url + MPT_ULITERAL("@") + mpt::ufmt::val(m_Revision);
}

mpt::ustring SourceInfo::GetStateString() const
{
	mpt::ustring retval;
	if(m_IsDirty)
	{
		retval += MPT_ULITERAL("+dirty");
	}
	if(m_HasMixedRevisions)
	{
		retval += MPT_ULITERAL("+mixed");
	}
	if(retval.empty())
	{
		retval += MPT_ULITERAL("clean");
	}
	if(m_IsPackage)
	{
		retval += MPT_ULITERAL("-pkg");
	}
	return retval;
}

SourceInfo SourceInfo::Current()
{
	return SourceInfo();
}



namespace Build {

bool IsReleasedBuild()
{
	return !(Version::Current().IsTestVersion() || IsDebugBuild() || Source::IsDirty() || Source::HasMixedRevisions());
}

bool IsDebugBuild()
{
	#ifdef _DEBUG
		return true;
	#else
		return false;
	#endif
}

mpt::ustring GetBuildDateString()
{
	mpt::ustring result;
	#ifdef MODPLUG_TRACKER
		#if defined(OPENMPT_BUILD_DATE)
			result = mpt::ToUnicode(mpt::CharsetASCII, OPENMPT_BUILD_DATE );
		#else
			result = mpt::ToUnicode(mpt::CharsetASCII, __DATE__ " " __TIME__ );
		#endif
	#else // !MODPLUG_TRACKER
		result = SourceInfo::Current().Date();
	#endif // MODPLUG_TRACKER
	return result;
}

static mpt::ustring GetBuildFlagsString()
{
	mpt::ustring retval;
	#ifdef MODPLUG_TRACKER
		if(Version::Current().IsTestVersion())
		{
			retval += MPT_ULITERAL(" TEST");
		}
	#endif // MODPLUG_TRACKER
	if(IsDebugBuild())
	{
		retval += MPT_ULITERAL(" DEBUG");
	}
	return retval;
}

mpt::ustring GetBuildFeaturesString()
{
	mpt::ustring retval;
	#ifdef LIBOPENMPT_BUILD
		retval = MPT_ULITERAL("")
		#if defined(MPT_CHARSET_WIN32)
			MPT_ULITERAL(" +WINAPI")
		#endif
		#if defined(MPT_CHARSET_ICONV)
			MPT_ULITERAL(" +ICONV")
		#endif
		#if defined(MPT_CHARSET_CODECVTUTF8)
			MPT_ULITERAL(" +CODECVTUTF8")
		#endif
		#if defined(MPT_CHARSET_INTERNAL)
			MPT_ULITERAL(" +INTERNALCHARSETS")
		#endif
		#if defined(MPT_WITH_ZLIB)
			MPT_ULITERAL(" +ZLIB")
		#endif
		#if defined(MPT_WITH_MINIZ)
			MPT_ULITERAL(" +MINIZ")
		#endif
		#if !defined(MPT_WITH_ZLIB) && !defined(MPT_WITH_MINIZ)
			MPT_ULITERAL(" -INFLATE")
		#endif
		#if defined(MPT_WITH_MPG123)
			MPT_ULITERAL(" +MPG123")
		#endif
		#if defined(MPT_WITH_MINIMP3)
			MPT_ULITERAL(" +MINIMP3")
		#endif
		#if defined(MPT_WITH_MEDIAFOUNDATION)
			MPT_ULITERAL(" +MF")
		#endif
		#if !defined(MPT_WITH_MPG123) && !defined(MPT_WITH_MINIMP3) && !defined(MPT_WITH_MEDIAFOUNDATION)
			MPT_ULITERAL(" -MP3")
		#endif
		#if defined(MPT_WITH_OGG) && defined(MPT_WITH_VORBIS) && defined(MPT_WITH_VORBISFILE)
			MPT_ULITERAL(" +VORBIS")
		#endif
		#if defined(MPT_WITH_STBVORBIS)
			MPT_ULITERAL(" +STBVORBIS")
		#endif
		#if !(defined(MPT_WITH_OGG) && defined(MPT_WITH_VORBIS) && defined(MPT_WITH_VORBISFILE)) && !defined(MPT_WITH_STBVORBIS)
			MPT_ULITERAL(" -VORBIS")
		#endif
		#if !defined(NO_PLUGINS)
			MPT_ULITERAL(" +PLUGINS")
		#else
			MPT_ULITERAL(" -PLUGINS")
		#endif
		#if !defined(NO_DMO)
			MPT_ULITERAL(" +DMO")
		#endif
		;
	#endif
	#ifdef MODPLUG_TRACKER
		#if (MPT_ARCH_BITS == 64)
			if (true
				&& (mpt::Windows::Version::GetMinimumKernelLevel() <= mpt::Windows::Version::WinXP64)
				&& (mpt::Windows::Version::GetMinimumAPILevel() <= mpt::Windows::Version::WinXP64)
			) {
				retval += MPT_ULITERAL(" WIN64OLD");
			}
		#elif (MPT_ARCH_BITS == 32)
			if (true
				&& (mpt::Windows::Version::GetMinimumKernelLevel() <= mpt::Windows::Version::WinXP)
				&& (mpt::Windows::Version::GetMinimumAPILevel() <= mpt::Windows::Version::WinXP)
			) {
				retval += MPT_ULITERAL(" WIN32OLD");
			}
		#endif
			retval += MPT_ULITERAL("")
		#if defined(UNICODE)
			MPT_ULITERAL(" UNICODE")
		#else
			MPT_ULITERAL(" ANSI")
		#endif
		#ifdef NO_VST
			MPT_ULITERAL(" NO_VST")
		#endif
		#ifdef NO_DMO
			MPT_ULITERAL(" NO_DMO")
		#endif
		#ifdef NO_PLUGINS
			MPT_ULITERAL(" NO_PLUGINS")
		#endif
		#ifndef MPT_WITH_ASIO
			MPT_ULITERAL(" NO_ASIO")
		#endif
		#ifndef MPT_WITH_DSOUND
			MPT_ULITERAL(" NO_DSOUND")
		#endif
			;
	#endif
	return retval;
}

mpt::ustring GetBuildCompilerString()
{
	mpt::ustring retval;
	#if MPT_COMPILER_GENERIC
		retval += MPT_USTRING("Generic C++11 Compiler");
	#elif MPT_COMPILER_MSVC
		#if defined(_MSC_FULL_VER) && defined(_MSC_BUILD) && (_MSC_BUILD > 0)
			retval += mpt::format(MPT_USTRING("Microsoft Compiler %1.%2.%3.%4"))
				( _MSC_FULL_VER / 10000000
				, mpt::ufmt::dec0<2>((_MSC_FULL_VER / 100000) % 100)
				, mpt::ufmt::dec0<5>(_MSC_FULL_VER % 100000)
				, mpt::ufmt::dec0<2>(_MSC_BUILD)
				);
		#elif defined(_MSC_FULL_VER)
			retval += mpt::format(MPT_USTRING("Microsoft Compiler %1.%2.%3"))
				( _MSC_FULL_VER / 10000000
				, mpt::ufmt::dec0<2>((_MSC_FULL_VER / 100000) % 100)
				, mpt::ufmt::dec0<5>(_MSC_FULL_VER % 100000)
				);
		#else
			retval += mpt::format(MPT_USTRING("Microsoft Compiler %1.%2"))(MPT_COMPILER_MSVC_VERSION / 100, MPT_COMPILER_MSVC_VERSION % 100);
		#endif
	#elif MPT_COMPILER_GCC
		retval += mpt::format(MPT_USTRING("GNU Compiler Collection %1.%2.%3"))(MPT_COMPILER_GCC_VERSION / 10000, (MPT_COMPILER_GCC_VERSION / 100) % 100, MPT_COMPILER_GCC_VERSION % 100);
	#elif MPT_COMPILER_CLANG
		retval += mpt::format(MPT_USTRING("Clang %1.%2.%3"))(MPT_COMPILER_CLANG_VERSION / 10000, (MPT_COMPILER_CLANG_VERSION / 100) % 100, MPT_COMPILER_CLANG_VERSION % 100);
	#else
		retval += MPT_USTRING("unknown");
	#endif
	return retval;
}

static mpt::ustring GetRevisionString()
{
	mpt::ustring result;
	if(Source::GetRevision() == 0)
	{
		return result;
	}
	result = MPT_USTRING("-r") + mpt::ufmt::val(Source::GetRevision());
	if(Source::HasMixedRevisions())
	{
		result += MPT_ULITERAL("!");
	}
	if(Source::IsDirty())
	{
		result += MPT_ULITERAL("+");
	}
	if(Source::IsPackage())
	{
		result += MPT_ULITERAL("p");
	}
	return result;
}

mpt::ustring GetVersionString(FlagSet<Build::Strings> strings)
{
	std::vector<mpt::ustring> result;
	if(strings[StringVersion])
	{
		result.push_back(mpt::ufmt::val(Version::Current()));
	}
	if(strings[StringRevision])
	{
		if(!IsReleasedBuild())
		{
			result.push_back(GetRevisionString());
		}
	}
	if(strings[StringBitness])
	{
		result.push_back(mpt::format(MPT_USTRING(" %1 bit"))(sizeof(void*)*8));
	}
	if(strings[StringSourceInfo])
	{
		const SourceInfo sourceInfo = SourceInfo::Current();
		if(!sourceInfo.GetUrlWithRevision().empty())
		{
			result.push_back(mpt::format(MPT_USTRING(" %1"))(sourceInfo.GetUrlWithRevision()));
		}
		if(!sourceInfo.Date().empty())
		{
			result.push_back(mpt::format(MPT_USTRING(" (%1)"))(sourceInfo.Date()));
		}
		if(!sourceInfo.GetStateString().empty())
		{
			result.push_back(mpt::format(MPT_USTRING(" %1"))(sourceInfo.GetStateString()));
		}
	}
	if(strings[StringBuildFlags])
	{
		if(!IsReleasedBuild())
		{
			result.push_back(GetBuildFlagsString());
		}
	}
	if(strings[StringBuildFeatures])
	{
		result.push_back(GetBuildFeaturesString());
	}
	return mpt::String::Trim(mpt::String::Combine<mpt::ustring>(result, MPT_USTRING("")));
}

mpt::ustring GetVersionStringPure()
{
	FlagSet<Build::Strings> strings;
	strings |= Build::StringVersion;
	strings |= Build::StringRevision;
	#ifdef MODPLUG_TRACKER
		strings |= Build::StringBitness;
	#endif
	return GetVersionString(strings);
}

mpt::ustring GetVersionStringSimple()
{
	FlagSet<Build::Strings> strings;
	strings |= Build::StringVersion;
	strings |= Build::StringRevision;
	strings |= Build::StringBuildFlags;
	return GetVersionString(strings);
}

mpt::ustring GetVersionStringExtended()
{
	FlagSet<Build::Strings> strings;
	strings |= Build::StringVersion;
	strings |= Build::StringRevision;
	#ifdef MODPLUG_TRACKER
		strings |= Build::StringBitness;
	#endif
	#ifndef MODPLUG_TRACKER
		strings |= Build::StringSourceInfo;
	#endif
	strings |= Build::StringBuildFlags;
	#ifdef MODPLUG_TRACKER
		strings |= Build::StringBuildFeatures;
	#endif
	return GetVersionString(strings);
}

mpt::ustring GetURL(Build::Url key)
{
	mpt::ustring result;
	switch(key)
	{
		case Url::Website:
			#ifdef LIBOPENMPT_BUILD
				result = MPT_USTRING("https://lib.openmpt.org/");
			#else
				result = MPT_USTRING("https://openmpt.org/");
			#endif
			break;
		case Url::Download:
			#ifdef MODPLUG_TRACKER
				result = IsReleasedBuild() ? MPT_USTRING("https://openmpt.org/download") : MPT_USTRING("https://builds.openmpt.org/builds/");
			#else
				result = MPT_USTRING("https://lib.openmpt.org/libopenmpt/download/");
			#endif
			break;
		case Url::Forum:
			result = MPT_USTRING("https://forum.openmpt.org/");
			break;
		case Url::Bugtracker:
			result = MPT_USTRING("https://bugs.openmpt.org/");
			break;
		case Url::Updates:
			result = MPT_USTRING("https://openmpt.org/download");
			break;
		case Url::TopPicks:
			result = MPT_USTRING("https://openmpt.org/top_picks");
			break;
	}
	return result;
}

mpt::ustring GetFullCreditsString()
{
	return mpt::ToUnicode(mpt::CharsetUTF8,
#ifdef MODPLUG_TRACKER
		"OpenMPT / ModPlug Tracker\n"
#else
		"libopenmpt (based on OpenMPT / ModPlug Tracker)\n"
#endif
		"\n"
		"Copyright \xC2\xA9 2004-2018 Contributors\n"
		"Copyright \xC2\xA9 1997-2003 Olivier Lapicque\n"
		"\n"
		"Contributors:\n"
		"Johannes Schultz (2008-2018)\n"
		"J\xC3\xB6rn Heusipp (2012-2018)\n"
		"Ahti Lepp\xC3\xA4nen (2005-2011)\n"
		"Robin Fernandes (2004-2007)\n"
		"Sergiy Pylypenko (2007)\n"
		"Eric Chavanon (2004-2005)\n"
		"Trevor Nunes (2004)\n"
		"Olivier Lapicque (1997-2003)\n"
		"\n"
		"Additional patch submitters:\n"
		"coda (http://coda.s3m.us/)\n"
		"kode54 (https://kode54.net/)\n"
		"Revenant (http://revenant1.net/)\n"
		"xaimus (http://xaimus.com/)\n"
		"\n"
		"Thanks to:\n"
		"\n"
		"Konstanty for the XMMS-ModPlug resampling implementation\n"
		"http://modplug-xmms.sourceforge.net/\n"
		"\n"
#ifdef MODPLUG_TRACKER
		"Stephan M. Bernsee for pitch shifting source code\n"
		"http://www.dspdimension.com/\n"
		"\n"
		"Aleksey Vaneev of Voxengo for r8brain sample rate converter\n"
		"https://github.com/avaneev/r8brain-free-src\n"
		"\n"
		"Olli Parviainen for SoundTouch Library (time stretching)\n"
		"http://www.surina.net/soundtouch/\n"
		"\n"
#endif
#ifndef NO_VST
		"Hermann Seib for his example VST Host implementation\n"
		"http://www.hermannseib.com/english/vsthost.htm\n"
		"\n"
#endif
		"Storlek for all the IT compatibility hints and testcases\n"
		"as well as the IMF, MDL, OKT and ULT loaders\n"
		"http://schismtracker.org/\n"
		"\n"
		"Sergei \"x0r\" Kolzun for various hints on Scream Tracker 2 compatibility\n"
		"https://github.com/viiri/st2play\n"
		"\n"
		"Laurent Cl\xc3\xA9vy for unofficial MO3 documentation and decompression code\n"
		"https://github.com/lclevy/unmo3\n"
		"\n"
		"Ben \"GreaseMonkey\" Russell for IT sample compression code\n"
		"https://github.com/iamgreaser/it2everything/\n"
		"\n"
		"Antti S. Lankila for Amiga resampler implementation\n"
		"https://bel.fi/alankila/modguide/interpolate.txt\n"
		"\n"
#ifdef MPT_WITH_ZLIB
		"Jean-loup Gailly and Mark Adler for zlib\n"
		"http://zlib.net/\n"
		"\n"
#endif
#ifdef MPT_WITH_MINIZ
		"Rich Geldreich et al. for miniz\n"
		"https://github.com/richgel999/miniz\n"
		"\n"
#endif
#ifdef MPT_WITH_LHASA
		"Simon Howard for lhasa\n"
		"https://fragglet.github.io/lhasa/\n"
		"\n"
#endif
#ifdef MPT_WITH_UNRAR
		"Alexander L. Roshal for UnRAR\n"
		"http://rarlab.com/\n"
		"\n"
#endif
#ifdef MPT_WITH_PORTAUDIO
		"PortAudio contributors\n"
		"http://www.portaudio.com/\n"
		"\n"
#endif
#ifdef MPT_WITH_RTAUDIO
		"Gary P. Scavone, McGill University\n"
		"https://www.music.mcgill.ca/~gary/rtaudio/\n"
		"\n"
#endif
#ifdef MPT_WITH_FLAC
		"Josh Coalson / Xiph.Org Foundation for libFLAC\n"
		"https://xiph.org/flac/\n"
		"\n"
#endif
#if defined(MPT_WITH_MPG123)
		"The mpg123 project for libmpg123\n"
		"https://mpg123.de/\n"
		"\n"
#endif
#ifdef MPT_WITH_MINIMP3
		"Lion (github.com/lieff) for minimp3\n"
		"https://github.com/lieff/minimp3/\n"
		"\n"
#endif
#ifdef MPT_WITH_STBVORBIS
		"Sean Barrett for stb_vorbis\n"
		"https://github.com/nothings/stb/\n"
		"\n"
#endif
#ifdef MPT_WITH_OGG
		"Xiph.Org Foundation for libogg\n"
		"https://xiph.org/ogg/\n"
		"\n"
#endif
#if defined(MPT_WITH_VORBIS) || defined(MPT_WITH_LIBVORBISFILE)
		"Xiph.Org Foundation for libvorbis\n"
		"https://xiph.org/vorbis/\n"
		"\n"
#endif
#if defined(MPT_WITH_OPUS)
		"Xiph.Org, Skype Limited, Octasic, Jean-Marc Valin, Timothy B. Terriberry,\n"
		"CSIRO, Gregory Maxwell, Mark Borgerding, Erik de Castro Lopo,\n"
		"Xiph.Org Foundation, Microsoft Corporation, Broadcom Corporation for libopus\n"
		"https://opus-codec.org/\n"
		"\n"
#endif
#if defined(MPT_WITH_OPUSFILE)
		"Xiph.Org Foundation and contributors for libopusfile\n"
		"https://opus-codec.org/\n"
		"\n"
#endif
#if defined(MPT_WITH_OPUSENC)
		"Xiph.Org Foundation, Jean-Marc Valin and contributors for libopusenc\n"
		"https://git.xiph.org/?p=libopusenc.git;a=summary\n"
		"\n"
#endif
#if defined(MPT_WITH_PICOJSON)
		"Cybozu Labs Inc. and Kazuho Oku et. al. for picojson\n"
		"https://github.com/kazuho/picojson\n"
		"\n"
#endif
#ifdef MODPLUG_TRACKER
		"Lennart Poettering and David Henningsson for RealtimeKit\n"
		"http://git.0pointer.net/rtkit.git/\n"
		"\n"
		"Gary P. Scavone for RtMidi\n"
		"https://www.music.mcgill.ca/~gary/rtmidi/\n"
		"\n"
		"Alexander Uckun for decimal input field\n"
		"http://www.codeproject.com/Articles/21257/_\n"
		"\n"
		"Nobuyuki for application and file icon\n"
		"https://twitter.com/nobuyukinyuu\n"
		"\n"
#endif
		"Daniel Collin (emoon/TBL) for providing test infrastructure\n"
		"https://twitter.com/daniel_collin\n"
		"\n"
		"The people at ModPlug forums for crucial contribution\n"
		"in the form of ideas, testing and support;\n"
		"thanks particularly to:\n"
		"33, 8bitbubsy, Anboi, BooT-SectoR-ViruZ, Bvanoudtshoorn\n"
		"christofori, cubaxd, Diamond, Ganja, Georg, Goor00,\n"
		"Harbinger, jmkz, KrazyKatz, LPChip, Nofold, Rakib, Sam Zen\n"
		"Skaven, Skilletaudio, Snu, Squirrel Havoc, Waxhead\n"
		"\n"
#ifndef NO_VST
		"VST PlugIn Technology by Steinberg Media Technologies GmbH\n"
		"\n"
#endif
#ifdef MPT_WITH_ASIO
		"ASIO Technology by Steinberg Media Technologies GmbH\n"
		"\n"
#endif
		);
}

mpt::ustring GetLicenseString()
{
	return MPT_UTF8(
		"The OpenMPT code is licensed under the BSD license." "\n"
		"" "\n"
		"Copyright (c) 2004-2018, OpenMPT contributors" "\n"
		"Copyright (c) 1997-2003, Olivier Lapicque" "\n"
		"All rights reserved." "\n"
		"" "\n"
		"Redistribution and use in source and binary forms, with or without" "\n"
		"modification, are permitted provided that the following conditions are met:" "\n"
		"    * Redistributions of source code must retain the above copyright" "\n"
		"      notice, this list of conditions and the following disclaimer." "\n"
		"    * Redistributions in binary form must reproduce the above copyright" "\n"
		"      notice, this list of conditions and the following disclaimer in the" "\n"
		"      documentation and/or other materials provided with the distribution." "\n"
		"    * Neither the name of the OpenMPT project nor the" "\n"
		"      names of its contributors may be used to endorse or promote products" "\n"
		"      derived from this software without specific prior written permission." "\n"
		"" "\n"
		"THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY" "\n"
		"EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED" "\n"
		"WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE" "\n"
		"DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY" "\n"
		"DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES" "\n"
		"(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;" "\n"
		"LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND" "\n"
		"ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT" "\n"
		"(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS" "\n"
		"SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE." "\n"
		);
}

} // namespace Build



OPENMPT_NAMESPACE_END

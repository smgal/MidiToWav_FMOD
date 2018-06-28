
#pragma once
#pragma comment(lib, "fmod64_vc.lib")

////////////////////////////////////////////////////////////////////////////////

#include "FModPlayer.h"

#include <fmod.hpp>
#include <fmod_errors.h>
#include <fmod_dsp.h>

#include <iostream>

////////////////////////////////////////////////////////////////////////////////

const int MAX_CHANNEL = 32;

struct FModPlayer::Impl
{
	Impl()
		: p_system(nullptr)
		, p_channel(nullptr)
		, p_sound(nullptr) { }

	FMOD::System* p_system;
	FMOD::Channel* p_channel;
	FMOD::Sound* p_sound;

	std::string dls_file_name;
};

////////////////////////////////////////////////////////////////////////////////

namespace
{
	void _CheckErrors(FMOD_RESULT result, const char* sz_function)
	{
		if (result != FMOD_OK)
			std::cout << "FMOD ERROR: " << sz_function << "(" << result << ") " << FMOD_ErrorString(result) << std::endl;
	}
}

#define CHECK_ERRORS(result) _CheckErrors(result, __FUNCTION__)

////////////////////////////////////////////////////////////////////////////////

FModPlayer::FModPlayer()
	: p_impl(new Impl)
{
	FMOD_RESULT result;

	result = FMOD::System_Create(&p_impl->p_system);
	CHECK_ERRORS(result);

	// version check
	{
		unsigned int lib_version;
		result = p_impl->p_system->getVersion(&lib_version);
		CHECK_ERRORS(result);

		if (lib_version < FMOD_VERSION)
			std::cout << "[FModPlayer] WARNING: Library version too low (lib: " << lib_version << ", curr: " << FMOD_VERSION << ")" << std::endl;
	}

	// output drive check
	{
		int num_drivers;
		result = p_impl->p_system->getNumDrivers(&num_drivers);
		CHECK_ERRORS(result);

		if (num_drivers == 0)
		{
			std::cout << "[FModPlayer] WARNING: No sound driver available" << std::endl;
			result = p_impl->p_system->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
			CHECK_ERRORS(result);
		}
	}

	result = p_impl->p_system->init(MAX_CHANNEL, FMOD_INIT_NORMAL, 0);
	CHECK_ERRORS(result);
}

FModPlayer::~FModPlayer()
{
	p_impl->p_sound->release();

	p_impl->p_system->close();
	p_impl->p_system->release();

	delete p_impl;
}

void FModPlayer::SetDLS(std::string file_name)
{
	p_impl->dls_file_name = file_name;
}

void FModPlayer::Play(std::string file_name)
{
	FMOD_CREATESOUNDEXINFO exinfo;
	memset(&exinfo, 0, sizeof(exinfo));
	exinfo.cbsize = sizeof(exinfo);

	if (!p_impl->dls_file_name.empty())
		exinfo.dlsname = p_impl->dls_file_name.c_str();

	FMOD_RESULT result = p_impl->p_system->createSound(file_name.c_str(), FMOD_DEFAULT, &exinfo, &p_impl->p_sound);
	CHECK_ERRORS(result);

	result = p_impl->p_system->playSound(p_impl->p_sound, 0, false, &p_impl->p_channel);
	CHECK_ERRORS(result);
}

////////////////////////////////////////////////////////////////////////////////

void ConvertMidiToWav(std::string mid_file_name, std::string dls_file_name, std::string wav_file_name)
{
	static std::string s_mid_file_name = mid_file_name;
	static std::string s_dls_file_name = dls_file_name;
	static std::string s_wav_file_name = wav_file_name;

	FMOD_CREATESOUNDEXINFO exinfo;
	memset(&exinfo, 0, sizeof(exinfo));
	exinfo.cbsize = sizeof(exinfo);
	exinfo.dlsname = s_dls_file_name.c_str();

	FMOD_RESULT result;
	int sampling_rate = 0;
	unsigned int sound_lenght = 0;
	unsigned int buffer_length = 0;

	FMOD::System* p_system = 0;
	{
		result = FMOD::System_Create(&p_system);
		CHECK_ERRORS(result);

		p_system->getSoftwareFormat(&sampling_rate, 0, nullptr);
		p_system->setOutput(FMOD_OUTPUTTYPE_WAVWRITER_NRT);
		p_system->init(MAX_CHANNEL, FMOD_INIT_STREAM_FROM_UPDATE, (void*)s_wav_file_name.c_str());
	}

	FMOD::Sound* p_sound = 0;
	{
		FMOD_REVERB_PROPERTIES reverb_prop = FMOD_PRESET_AUDITORIUM;
		p_system->setReverbProperties(0, &reverb_prop);

		result = p_system->createSound(mid_file_name.c_str(), FMOD_DEFAULT, &exinfo, &p_sound);
		CHECK_ERRORS(result);

		p_sound->getLength(&sound_lenght, FMOD_TIMEUNIT_MS);
		p_system->getDSPBufferSize(&buffer_length, 0);
	}

	FMOD::Channel* p_channel = 0;
	{
		result = p_system->playSound(p_sound, 0, false, &p_channel);
		CHECK_ERRORS(result);
	}

	double index = 0;
	while (index < sound_lenght)
	{
		result = p_system->update();
		index += ((double)buffer_length / (double)sampling_rate) * 1000;
	}

	p_sound->release();

	p_system->close();
	p_system->release();
}

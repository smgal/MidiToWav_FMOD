
#ifndef __FMOD_PLAYER_H__
#define __FMOD_PLAYER_H__

#include <string>

class FModPlayer
{
public:
	FModPlayer();
	~FModPlayer();

	void SetDLS(std::string file_name);
	void Play(std::string file_name);

private:
	struct Impl;
	Impl* p_impl;
};

void ConvertMidiToWav(std::string mid_file_name, std::string dls_file_name, std::string wav_file_name);

#endif


#include "FModPlayer.h"

int main()
{
#if 1
	ConvertMidiToWav("Res/Level42_SomethingAboutYou.mid", "DLS_Level42_SomethingAboutYou.wav", "Res/8Rock11_DLS1.dls");

#else
	FModPlayer player;

	player.SetDLS("Res/8Rock11_DLS1.dls");
	player.Play("Res/Level42_SomethingAboutYou.mid");

	system("pause");

#endif

    return 0;
}

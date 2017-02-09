#include "VapourSynth.h"

extern auto mvsuperRegister(VSRegisterFunction registerFunc, VSPlugin *plugin)->void;
extern auto mvanalyzeRegister(VSRegisterFunction registerFunc, VSPlugin *plugin)->void;
extern auto mvanalyseRegister(VSRegisterFunction registerFunc, VSPlugin *plugin)->void;
extern auto mvdegrainsRegister(VSRegisterFunction registerFunc, VSPlugin *plugin)->void;
extern auto mvcompensateRegister(VSRegisterFunction registerFunc, VSPlugin *plugin)->void;
extern auto mvrecalculateRegister(VSRegisterFunction registerFunc, VSPlugin *plugin)->void;
extern auto mvmaskRegister(VSRegisterFunction registerFunc, VSPlugin *plugin)->void;
extern auto mvfinestRegister(VSRegisterFunction registerFunc, VSPlugin *plugin)->void;
extern auto mvflowRegister(VSRegisterFunction registerFunc, VSPlugin *plugin)->void;
extern auto mvflowblurRegister(VSRegisterFunction registerFunc, VSPlugin *plugin)->void;
extern auto mvflowinterRegister(VSRegisterFunction registerFunc, VSPlugin *plugin)->void;
extern auto mvflowfpsRegister(VSRegisterFunction registerFunc, VSPlugin *plugin)->void;
extern auto mvblockfpsRegister(VSRegisterFunction registerFunc, VSPlugin *plugin)->void;
extern auto mvscdetectionRegister(VSRegisterFunction registerFunc, VSPlugin *plugin)->void;

VS_EXTERNAL_API(void) VapourSynthPluginInit(VSConfigPlugin configFunc, VSRegisterFunction registerFunc, VSPlugin *plugin) {
	configFunc("com.nodame.mvsf", "mvsf", "MVTools Single Precision", VAPOURSYNTH_API_VERSION, 1, plugin);
	mvsuperRegister(registerFunc, plugin);
	mvanalyzeRegister(registerFunc, plugin);
	mvanalyseRegister(registerFunc, plugin);
	mvdegrainsRegister(registerFunc, plugin);
	mvcompensateRegister(registerFunc, plugin);
	mvrecalculateRegister(registerFunc, plugin);
	mvmaskRegister(registerFunc, plugin);
	mvfinestRegister(registerFunc, plugin);
	mvflowRegister(registerFunc, plugin);
	mvflowblurRegister(registerFunc, plugin);
	mvflowinterRegister(registerFunc, plugin);
	mvflowfpsRegister(registerFunc, plugin);
	mvblockfpsRegister(registerFunc, plugin);
	mvscdetectionRegister(registerFunc, plugin);
}

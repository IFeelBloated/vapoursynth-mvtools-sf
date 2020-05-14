#include "Include/Interface.hxx"
#include "MVSuper.hxx"
#include "MVAnalyze.hxx"
#include "MVDegrain.hxx"
#include "MVCompensate.hxx"
#include "MVRecalculate.hxx"
#include "MVMask.hxx"
#include "MVFinest.hxx"
#include "MVFlow.hxx"
#include "MVFlowBlur.hxx"
#include "MVFlowInter.hxx"
#include "MVFlowFPS.hxx"
#include "MVBlockFPS.hxx"
#include "MVSCDetection.hxx"

VS_EXTERNAL_API(void) VapourSynthPluginInit(VSConfigPlugin configFunc, VSRegisterFunction registerFunc, VSPlugin *plugin) {
	VaporGlobals::Identifier = "com.zonked.mvsf";
	VaporGlobals::Namespace = "mvsf";
	VaporGlobals::Description = "MVTools Single Precision";
	VaporInterface::RegisterPlugin(configFunc, plugin);
	mvsuperRegister(registerFunc, plugin);
	mvanalyzeRegister(registerFunc, plugin);
	mvdegrainRegister(registerFunc, plugin);
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

#pragma once
#include "Core.hxx"
#include "Buffer.hxx"

namespace VaporInterface {
	template<typename FilterType>
	auto Initialize(auto, auto, auto InstanceData, auto Node, auto Core, auto...) {
		auto Data = reinterpret_cast<FilterType*>(*InstanceData);
		auto VideoInfo = Data->RegisterVideoInfo(VaporCore{ .Instance = Core });
		VaporGlobals::API->setVideoInfo(&VideoInfo, 1, Node);
	}

	template<typename FilterType>
	auto Delete(auto InstanceData, auto...) {
		auto Data = reinterpret_cast<FilterType*>(InstanceData);
		delete Data;
	}

	template<typename FilterType>
	auto Evaluate(auto Index, auto ActivationReason, auto InstanceData, auto, auto FrameContext, auto Core, auto...) {
		auto Data = reinterpret_cast<FilterType*>(*InstanceData);
		auto NullFrame = static_cast<const VSFrameRef*>(nullptr);
		if (ActivationReason == VSActivationReason::arInitial)
			Data->RequestReferenceFrames(Index, FrameContext);
		else if (ActivationReason == VSActivationReason::arAllFramesReady)
			return Data->DrawFrame(Index, VaporCore{ .Instance = Core }, VaporFrameContext<FilterType>{ .Context = FrameContext });
		return NullFrame;
	}

	template<typename FilterType>
	auto Create(auto InputMap, auto OutputMap, auto, auto Core, auto...) {
		auto Data = FilterType{};
		auto Console = Controller<FilterType>{ OutputMap };
		auto SelfEvaluator = [&](auto EvaluatedMap, auto InstanceData) {
			auto AuxiliaryMap = VaporGlobals::API->createMap();
			auto AssumedMultithreadingMode = VSFilterMode::fmParallel;
			auto AssumedCacheFlag = 0;
			if constexpr (hasattr(Data, MultithreadingMode))
				AssumedMultithreadingMode = FilterType::MultithreadingMode;
			if constexpr (hasattr(Data, CacheFlag))
				AssumedCacheFlag = FilterType::CacheFlag;
			if constexpr (hasattr(Data, DrawFrame))
				VaporGlobals::API->createFilter(AuxiliaryMap, EvaluatedMap, FilterType::Name, Initialize<FilterType>, Evaluate<FilterType>, Delete<FilterType>, AssumedMultithreadingMode, AssumedCacheFlag, InstanceData, Core);
			VaporGlobals::API->freeMap(AuxiliaryMap);
		};
		auto SelfInvoker = [&](auto&& ...Arguments) {
			auto InstanceData = new FilterType{};
			auto ArgumentMap = VaporGlobals::API->createMap();
			auto EvaluatedMap = VaporGlobals::API->createMap();
			if constexpr (sizeof...(Arguments) != 0)
				VaporPlugin::VaporFilter::ForwardArguments(ArgumentMap, Forward(Arguments)...);
			InstanceData->Initialize(ArgumentList{ ArgumentMap }, Console);
			SelfEvaluator(EvaluatedMap, InstanceData);
			auto EvaluatedClip = VaporGlobals::API->propGetNode(EvaluatedMap, "clip", 0, nullptr);
			VaporGlobals::API->freeMap(EvaluatedMap);
			VaporGlobals::API->freeMap(ArgumentMap);
			return Clip{ EvaluatedClip };
		};
		if (auto InitializationStatus = Data.Initialize(ArgumentList{ InputMap }, Console); InitializationStatus == false)
			return;
		if constexpr (hasattr(Data, RegisterInvokingSequence))
			Data.RegisterInvokingSequence(VaporCore{ .Instance = Core }, SelfInvoker, Console);
		else
			SelfEvaluator(OutputMap, new FilterType{ std::move(Data) });
	}

	template<typename FilterType>
	auto RegisterFilter(auto Broker, auto Plugin) {
		Broker(FilterType::Name, FilterType::Parameters, Create<FilterType>, nullptr, Plugin);
	}

	auto RegisterPlugin(auto Broker, auto Plugin) {
		VaporGlobals::API = getVapourSynthAPI(VAPOURSYNTH_API_VERSION);
		Broker(VaporGlobals::Identifier, VaporGlobals::Namespace, VaporGlobals::Description, VAPOURSYNTH_API_VERSION, 1, Plugin);
	}
}
#pragma once

#include <string>
#include <Difu/Particles/ParticleEmitter.h>

namespace ParticleSerializer 
{
	void Serialize(const std::string& filename, const std::string& emitter_name, const ParticleEmitter& emitter);
	void Deserialize(const std::string& filename, ParticleEmitter* emitter);
}

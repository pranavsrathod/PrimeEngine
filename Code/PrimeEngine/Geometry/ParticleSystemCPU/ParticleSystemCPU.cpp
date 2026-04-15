#define NOMINMAX

// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// immediate include 
#include "ParticleSystemCPU.h"
#include "ParticleBufferCPU.h"
#include <PrimeEngine/Scene/CameraManager.h>
#include <PrimeEngine/Scene/CameraSceneNode.h>


namespace PE {



	void ParticleSystemCPU::create(Matrix4x4 base)
	{
		m_base = Matrix4x4(base);
		createParticleBuffer();
	}

	void ParticleSystemCPU::createParticleBuffer() {
		m_hParticleBufferCPU = Handle("PARTICLE_BUFFER_CPU", sizeof(ParticleBufferCPU<ParticleCPU>));
		ParticleBufferCPU<ParticleCPU>* pBufferCPU = new (m_hParticleBufferCPU) ParticleBufferCPU<ParticleCPU>(*m_pContext, m_arena);
		PrimitiveTypes::Int32 maxParticleSize = m_particleTemplate.m_duration * m_particleTemplate.m_rate;
		pBufferCPU->m_values.reset(maxParticleSize);

		ParticleCPU newParticle = {m_base, m_particleTemplate.m_size, m_base.getN(), m_particleTemplate.m_duration};
		pBufferCPU->m_values.add(newParticle);

		m_hMaterialSetCPU = Handle("MATERIAL_SET_CPU", sizeof(MaterialSetCPU));
		MaterialSetCPU* pmscpu = new(m_hMaterialSetCPU) MaterialSetCPU(*m_pContext, m_arena);
		pmscpu->createSetWithOneTexturedMaterial(m_particleTemplate.m_texture, "Default");
        PEINFO("ParticleSystemCPU: Created particle buffer with %d slots\n", maxParticleSize);
	}
    Vector3 ParticleSystemCPU::generateVelocity()
    {
        if (m_systemType == PE::Components::ParticleMesh::ParticleSystemType::PST_FOUNTAIN)
        {
            // Mostly upward with some random sideways drift
            float spread = 0.5f; // tweak for wider/narrower fountain
            float x = ((float)rand() / RAND_MAX - 0.5f) * spread;
            float z = ((float)rand() / RAND_MAX - 0.5f) * spread;
            float y = 1.0f; // strong upward

            Vector3 dir(x, y, z);
            dir.normalize();

            return dir * m_particleTemplate.m_speed; // speed comes from template
        }
        else if (m_systemType == PE::Components::ParticleMesh::ParticleSystemType::PST_SPIRAL)
        {
            PrimitiveTypes::Float32 theta = ((float)rand() / RAND_MAX) * 2.0f * PrimitiveTypes::Constants::c_Pi_F32;

            float x = cosf(theta);
            float z = sinf(theta);
            float y = ((float)rand() / RAND_MAX) * 0.5f; // small upward drift

            Vector3 dir(x, y, z);
            dir.normalize();

            return dir * m_particleTemplate.m_speed;
        }
        else if (m_systemType == PE::Components::ParticleMesh::ParticleSystemType::PST_FIRE)
        {
            float spread = 0.3f; // narrower spread for fire
            float x = ((float)rand() / RAND_MAX - 0.5f) * spread;
            float z = ((float)rand() / RAND_MAX - 0.5f) * spread;
            float y = 1.0f; // strong upward

            Vector3 dir(x, y, z);
            dir.normalize();

            return dir * m_particleTemplate.m_speed;
        }
        else
        {
            // Regular spherical explosion
            PrimitiveTypes::Float32 z = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
            PrimitiveTypes::Float32 theta = ((float)rand() / RAND_MAX) * 2.0f * PrimitiveTypes::Constants::c_Pi_F32;
            PrimitiveTypes::Float32 r = sqrtf(1.0f - z * z);

            PrimitiveTypes::Float32 x = r * cosf(theta);
            PrimitiveTypes::Float32 y = r * sinf(theta);

            Vector3 dir(x, y, z);
            dir.normalize();
            return dir * m_particleTemplate.m_speed;
        }
    }

    void ParticleSystemCPU::updateParticleBuffer(PrimitiveTypes::Float32 time)
    {
        /*PEINFO("Updating particles");*/
        ParticleBufferCPU<ParticleCPU>* ppbcpu;
        if (!m_hParticleBufferCPU.isValid())
        {
            m_hParticleBufferCPU = Handle("PARTICLE_BUFFER_CPU", sizeof(ParticleBufferCPU<ParticleCPU>));
            ppbcpu = new(m_hParticleBufferCPU) ParticleBufferCPU<ParticleCPU>(*m_pContext, m_arena);
            const PrimitiveTypes::Int32 maxParticleSize = m_particleTemplate.m_duration * m_particleTemplate.m_rate;
            ppbcpu->m_values.reset(maxParticleSize);
        }
        else
        {
            ppbcpu = m_hParticleBufferCPU.getObject<ParticleBufferCPU<ParticleCPU>>();
        }

        m_pastTime += time;
        int totalParticles = m_pastTime * m_particleTemplate.m_rate;

        for (int j = 0; j < ppbcpu->m_values.m_size; j++)
        {
            ppbcpu->m_values[j].m_age -= time;
            if (m_systemType == PE::Components::ParticleMesh::ParticleSystemType::PST_FIRE)
            {
                float gravity = 5.0f; // smaller gravity pull for fire
                ppbcpu->m_values[j].m_velocity.m_y -= gravity * time;
            }
            if (m_systemType == PE::Components::ParticleMesh::ParticleSystemType::PST_SPIRAL)
            {
                float spiralSpeed = 5.0f; // tweak as needed
                Vector3 vel = ppbcpu->m_values[j].m_velocity;

                float angle = spiralSpeed * time;
                float cosA = cosf(angle);
                float sinA = sinf(angle);

                float newX = vel.m_x * cosA - vel.m_z * sinA;
                float newZ = vel.m_x * sinA + vel.m_z * cosA;

                ppbcpu->m_values[j].m_velocity.m_x = newX;
                ppbcpu->m_values[j].m_velocity.m_z = newZ;
            }
            if (m_systemType == PE::Components::ParticleMesh::ParticleSystemType::PST_FOUNTAIN)
            {
                float gravity = 9.8f;
                ppbcpu->m_values[j].m_velocity.m_y -= gravity * time;
            }
            if (ppbcpu->m_values[j].m_age <= 0)
            {
                if (m_systemType == PE::Components::ParticleMesh::ParticleSystemType::PST_SPHERE || m_systemType == PE::Components::ParticleMesh::ParticleSystemType::PST_FOUNTAIN || m_systemType == PE::Components::ParticleMesh::ParticleSystemType::PST_SPIRAL || m_systemType == PE::Components::ParticleMesh::ParticleSystemType::PST_FIRE)
                {
                    ParticleCPU newParticle = { m_base,
                                                m_particleTemplate.m_size,
                                                generateVelocity(),
                                                m_particleTemplate.m_duration };
                    ppbcpu->m_values[j] = newParticle;
                }
            }
            else
            {
                Vector3 curPos = ppbcpu->m_values[j].m_base.getPos();
                curPos += time * ppbcpu->m_values[j].m_velocity;
                ppbcpu->m_values[j].m_base.setPos(curPos);
            }
        }

        if ((m_systemType == PE::Components::ParticleMesh::ParticleSystemType::PST_SPHERE || m_systemType == PE::Components::ParticleMesh::ParticleSystemType::PST_FOUNTAIN || m_systemType == PE::Components::ParticleMesh::ParticleSystemType::PST_SPIRAL || m_systemType == PE::Components::ParticleMesh::ParticleSystemType::PST_FIRE) &&
            totalParticles < ppbcpu->m_values.m_capacity && totalParticles > ppbcpu->m_values.m_size)
        {
            PrimitiveTypes::Int16 partCount = totalParticles - ppbcpu->m_values.m_size;
            for (int i = 0; i < partCount; i++)
            {
                ParticleCPU newParticle = { m_base,
                                            m_particleTemplate.m_size,
                                            generateVelocity(),
                                            m_particleTemplate.m_duration };
                ppbcpu->m_values.add(newParticle);
            }
        }

        for (int j = 0; j < ppbcpu->m_values.m_size; j++)
        {
            Components::CameraSceneNode* pcam = Components::CameraManager::Instance()->getActiveCamera()->getCamSceneNode();
            Vector3 cameraFront = pcam->m_worldTransform.getN();
            Vector3 cameraRight = pcam->m_worldTransform.getU();
            Vector3 cameraUp = pcam->m_worldTransform.getV();

            ppbcpu->m_values[j].m_base.setN(cameraFront);
            ppbcpu->m_values[j].m_base.setU(cameraRight);
            ppbcpu->m_values[j].m_base.setV(cameraUp);
        }
    }

   /* void ParticleSystemCPU::updateParticleBuffer(PrimitiveTypes::Float32 time)
    {
        PEINFO("Updating particles");
        ParticleBufferCPU<ParticleCPU>* ppbcpu;
        if (!m_hParticleBufferCPU.isValid())
        {
            m_hParticleBufferCPU = Handle("PARTICLE_BUFFER_CPU", sizeof(ParticleBufferCPU<ParticleCPU>));
            ppbcpu = new(m_hParticleBufferCPU) ParticleBufferCPU<ParticleCPU>(*m_pContext, m_arena);
            const PrimitiveTypes::Int32 maxParticleSize = m_particleTemplate.m_duration * m_particleTemplate.m_rate;
            ppbcpu->m_values.reset(maxParticleSize);
        }
        else
        {
            ppbcpu = m_hParticleBufferCPU.getObject<ParticleBufferCPU<ParticleCPU>>();
        }

        m_pastTime += time;
        int totalParticles = m_pastTime * m_particleTemplate.m_rate;

        for (int j = 0; j < ppbcpu->m_values.m_size; j++)
        {
            ppbcpu->m_values[j].m_age -= time;
            if (ppbcpu->m_values[j].m_age <= 0)
            {
                ParticleCPU newParticle = { Matrix4x4(),
                                            m_particleTemplate.m_size,
                                            generateVelocity(),
                                            m_particleTemplate.m_duration };
                ppbcpu->m_values[j] = newParticle;
            }
            else
            {
                Vector3 curPos = ppbcpu->m_values[j].m_base.getPos();
                curPos += time * ppbcpu->m_values[j].m_velocity;
                ppbcpu->m_values[j].m_base.setPos(curPos);
            }
        }

        if (totalParticles < ppbcpu->m_values.m_capacity && totalParticles > ppbcpu->m_values.m_size)
        {
            PrimitiveTypes::Int16 partCount = totalParticles - ppbcpu->m_values.m_size;
            for (int i = 0; i < partCount; i++)
            {
                ParticleCPU newParticle = { Matrix4x4(),
                                            m_particleTemplate.m_size,
                                            generateVelocity(),
                                            m_particleTemplate.m_duration };
                ppbcpu->m_values.add(newParticle);
            }
        }

        for (int j = 0; j < ppbcpu->m_values.m_size; j++)
        {
            Components::CameraSceneNode* pcam = Components::CameraManager::Instance()->getActiveCamera()->getCamSceneNode();
            Vector3 cameraFront = pcam->m_worldTransform.getN();
            Vector3 cameraRight = pcam->m_worldTransform.getU();
            Vector3 cameraUp = pcam->m_worldTransform.getV();

            ppbcpu->m_values[j].m_base.setN(cameraFront);
            ppbcpu->m_values[j].m_base.setU(cameraRight);
            ppbcpu->m_values[j].m_base.setV(cameraUp);
        }
    }*/


}; // namespace PE {

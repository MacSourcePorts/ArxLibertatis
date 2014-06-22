/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#include "graphics/spells/Spells03.h"

#include <climits>

#include "core/GameTime.h"

#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/Particle.h"
#include "graphics/particle/ParticleParams.h"
#include "graphics/spells/Spells05.h"

#include "scene/Object.h"
#include "scene/Interactive.h"

CFireBall::CFireBall()
	: CSpellFx()
	, m_createBallDuration(2000)
{
	
	eSrc = Vec3f_ZERO;
	
	SetDuration(2000);
	ulCurrentTime = ulDuration + 1;
	
	bExplo = false;
}

CFireBall::~CFireBall()
{
}

void CFireBall::SetTTL(unsigned long aulTTL)
{
	unsigned long t = ulCurrentTime;
	ulDuration = min(ulCurrentTime + aulTTL, ulDuration);
	SetDuration(ulDuration);
	ulCurrentTime = t;
	
	lLightId = InvalidLightHandle;
}

void CFireBall::Create(Vec3f aeSrc, float afBeta, float afAlpha)
{
	SetDuration(ulDuration);
	
	eSrc.x = aeSrc.x - std::sin(radians(afBeta)) * 60;
	eSrc.y = aeSrc.y;
	eSrc.z = aeSrc.z + std::cos(radians(afBeta)) * 60;

	eMove.x = - std::sin(radians(afBeta)) * 80 * cos(radians(MAKEANGLE(afAlpha)));
	eMove.y = sin(radians(MAKEANGLE(afAlpha))) * 80;
	eMove.z = + std::cos(radians(afBeta)) * 80 * cos(radians(MAKEANGLE(afAlpha)));
	
	// Light
	lLightId = InvalidLightHandle;
	eCurPos = eSrc;
}

void CFireBall::Update(float timeDelta)
{
	ulCurrentTime += timeDelta;

	if(ulCurrentTime <= m_createBallDuration) {
		
		float afAlpha = 0.f;
		float afBeta = 0.f;
	
		SpellBase * spell = spells[spellinstance];
		
		if(spell->m_caster == PlayerEntityHandle) {
			afBeta = player.angle.getPitch();
			afAlpha = player.angle.getYaw();
			long idx = GetGroupOriginByName(entities[spell->m_caster]->obj, "chest");

			if(idx) {
				eCurPos.x = entities[spell->m_caster]->obj->vertexlist3[idx].v.x - std::sin(radians(afBeta)) * 60;
				eCurPos.y = entities[spell->m_caster]->obj->vertexlist3[idx].v.y;
				eCurPos.z = entities[spell->m_caster]->obj->vertexlist3[idx].v.z + std::cos(radians(afBeta)) * 60;
			} else {
				eCurPos.x = player.pos.x - std::sin(radians(afBeta)) * 60;
				eCurPos.y = player.pos.y;
				eCurPos.z = player.pos.z + std::cos(radians(afBeta)) * 60;
			}
		} else {
			afBeta = entities[spell->m_caster]->angle.getPitch();

			eCurPos.x = entities[spell->m_caster]->pos.x - std::sin(radians(afBeta)) * 60;
			eCurPos.y = entities[spell->m_caster]->pos.y;
			eCurPos.z = entities[spell->m_caster]->pos.z + std::cos(radians(afBeta)) * 60;

			if ((ValidIONum(spell->m_caster))
			        && (entities[spell->m_caster]->ioflags & IO_NPC))
			{
				eCurPos.x -= std::sin(radians(entities[spell->m_caster]->angle.getPitch())) * 30.f;
				eCurPos.y -= 80.f;
				eCurPos.z += std::cos(radians(entities[spell->m_caster]->angle.getPitch())) * 30.f;
			}
			
			Entity * io = entities[spell->m_caster];

			if(ValidIONum(io->targetinfo)) {
				Vec3f * p1 = &eCurPos;
				Vec3f p2 = entities[io->targetinfo]->pos;
				p2.y -= 60.f;
				afAlpha = 360.f - (degrees(getAngle(p1->y, p1->z, p2.y, p2.z + glm::distance(Vec2f(p2.x, p2.z), Vec2f(p1->x, p1->z))))); //alpha entre orgn et dest;
			}
		}

		eMove.x = - std::sin(radians(afBeta)) * 100 * cos(radians(MAKEANGLE(afAlpha)));
		eMove.y = sin(radians(MAKEANGLE(afAlpha))) * 100;
		eMove.z = + std::cos(radians(afBeta)) * 100 * cos(radians(MAKEANGLE(afAlpha)));
	}
	
	eCurPos += eMove * (timeDelta * 0.0045f);
}

void CFireBall::Render() {

}

CIceProjectile::~CIceProjectile()
{
	stite_count--;

	if(stite && (stite_count <= 0)) {
		stite_count = 0;
		delete stite;
		stite = NULL;
	}

	smotte_count--;

	if(smotte && (smotte_count <= 0)) {
		smotte_count = 0;
		delete smotte;
		smotte = NULL;
	}
}

CIceProjectile::CIceProjectile()
	: fColor(1.f)
{
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;

	iNumber = MAX_ICE;

	tex_p1 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_bluepouf");

	if (!stite)
		stite = LoadTheObj("graph/obj3d/interactive/fix_inter/stalagmite/stalagmite.teo");

	stite_count++;

	if (!smotte)
		smotte = LoadTheObj("graph/obj3d/interactive/fix_inter/stalagmite/motte.teo");

	smotte_count++;

	iMax = MAX_ICE;
	fStep = 20;
}

void CIceProjectile::Create(Vec3f aeSrc, float afBeta, float fLevel, EntityHandle caster)
{
	iMax = (int)(30 + fLevel * 5.2f);
	
	SetDuration(ulDuration);
	
	float fBetaRad = radians(afBeta);
	float fBetaRadCos = (float) cos(fBetaRad);
	float fBetaRadSin = (float) sin(fBetaRad);
	
	Vec3f s, e, h;

	s.x					= aeSrc.x;
	s.y					= aeSrc.y - 100;
	s.z					= aeSrc.z;

	float fspelldist	= static_cast<float>(iMax * 15);

	fspelldist = min(fspelldist, 200.0f);
	fspelldist = max(fspelldist, 450.0f);
	e.x = aeSrc.x - fBetaRadSin * fspelldist;
	e.y = aeSrc.y - 100;
	e.z = aeSrc.z + fBetaRadCos * fspelldist;

	float fd;

	if(!Visible(s, e, NULL, &h)) {
		e.x = h.x + fBetaRadSin * 20;
		e.y = h.y;
		e.z = h.z - fBetaRadCos * 20;
	}

	fd = fdist(s, e);

	float fCalc = ulDuration * (fd / fspelldist);
	SetDuration(checked_range_cast<unsigned long>(fCalc));

	float fDist = (fd / fspelldist) * iMax ;

	iNumber = checked_range_cast<int>(fDist);

	int end = iNumber / 2;
	tv1a[0] = s + Vec3f(0.f, 100.f, 0.f);
	tv1a[end] = e + Vec3f(0.f, 100.f, 0.f);

	Split(tv1a, 0, end, 80, 0.5f, 0, 1, 80, 0.5f);

	for(int i = 0; i < iNumber; i++) {
		float t = rnd();

		if (t < 0.5f)
			tType[i] = 0;
		else
			tType[i] = 1;

		tSize[i] = Vec3f_ZERO;
		tSizeMax[i] = randomVec() + Vec3f(0.f, 0.2f, 0.f);

		Vec3f minSize;
		if(tType[i] == 0) {
			minSize = Vec3f(1.2f, 1.f, 1.2f);
		} else {
			minSize = Vec3f(0.4f, 0.3f, 0.4f);
		}
		
		tSizeMax[i] = glm::max(tSizeMax[i], minSize);
		
		int iNum = static_cast<int>(i / 2);

		if(tType[i] == 0) {
			tPos[i].x = tv1a[iNum].x + frand2() * 80;
			tPos[i].y = tv1a[iNum].y;
			tPos[i].z = tv1a[iNum].z + frand2() * 80;
		} else {
			tPos[i].x = tv1a[iNum].x + frand2() * 40;
			tPos[i].y = tv1a[iNum].y;
			tPos[i].z = tv1a[iNum].z + frand2() * 40;
		}
		
		DamageParameters damage;
		damage.pos = tPos[i];
		damage.radius = 60.f;
		damage.damages = 0.1f * fLevel;
		damage.area = DAMAGE_FULL;
		damage.duration = ulDuration;
		damage.source = caster;
		damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE;
		damage.type = DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_COLD;
		DamageCreate(damage);
	}

	fColor = 1;
}

void CIceProjectile::Update(float timeDelta)
{
	ulCurrentTime += timeDelta;

	if(ulDuration - ulCurrentTime < 1000) {
		fColor = (ulDuration - ulCurrentTime) * ( 1.0f / 1000 );

		for(int i = 0; i < iNumber; i++) {
			tSize[i].y *= fColor;
		}
	}
}

void CIceProjectile::Render()
{
	int i = 0;

	if(ulCurrentTime >= ulDuration)
		return;

	GRenderer->SetCulling(Renderer::CullCW);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendInvDstColor, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	float fOneOnDuration = 1.f / (float)(ulDuration);
	iMax = (int)((iNumber * 2) * fOneOnDuration * ulCurrentTime);

	if(iMax > iNumber)
		iMax = iNumber;

	for(i = 0; i < iMax; i++) {
		
		tSize[i] += Vec3f(0.1f);
		tSize[i] = glm::clamp(tSize[i], Vec3f(0.f), tSizeMax[i]);
		
		Anglef stiteangle;
		Vec3f stitepos;
		Vec3f stitescale;
		Color3f stitecolor;

		stiteangle.setPitch((float) cos(radians(tPos[i].x)) * 360);
		stiteangle.setYaw(0);
		stiteangle.setRoll(0);
		stitepos = tPos[i];

		float tt;
		tt = tSizeMax[i].y * fColor;
		stitecolor.g = stitecolor.r = tt * 0.7f;
		stitecolor.b = tt * 0.9f;

		if(stitecolor.r > 1)
			stitecolor.r = 1;

		if(stitecolor.g > 1)
			stitecolor.g = 1;

		if(stitecolor.b > 1)
			stitecolor.b = 1;

		stitescale = tSize[i];

		if(tType[i] == 0)
			Draw3DObject(smotte, stiteangle, stitepos, stitescale, stitecolor);
		else
			Draw3DObject(stite, stiteangle, stitepos, stitescale, stitecolor);
	}
	
	for(i = 0; i < min(iNumber, iMax + 1); i++) {
		
		float t = rnd();
		if(t < 0.01f) {
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = tPos[i] + randomVec(-5.f, 5.f);
				pd->move = randomVec(-2.f, 2.f);
				pd->siz = 20.f;
				float t = min(2000.f + rnd() * 2000.f,
				              ulDuration - ulCurrentTime + 500.0f * rnd());
				pd->tolive = checked_range_cast<unsigned long>(t);
				pd->tc = tex_p2;
				pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				pd->fparam = 0.0000001f;
				pd->rgb = Color3f(0.7f, 0.7f, 1.f);
			}
			
		} else if(t > 0.095f) {
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = tPos[i] + randomVec(-5.f, 5.f) - Vec3f(0.f, 50.f, 0.f);
				pd->move = Vec3f(0.f, 2.f - 4.f * rnd(), 0.f);
				pd->siz = 0.5f;
				float t = min(2000.f + rnd() * 1000.f,
				              ulDuration - ulCurrentTime + 500.0f * rnd());
				pd->tolive = checked_range_cast<unsigned long>(t);
				pd->tc = tex_p1;
				pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				pd->fparam = 0.0000001f;
				pd->rgb = Color3f(0.7f, 0.7f, 1.f);
			}
			
		}
	}
}

//-----------------------------------------------------------------------------
// SPEED
//-----------------------------------------------------------------------------
CSpeed::~CSpeed()
{
	for(size_t i = 0; i < m_trails.size(); i++) {
		delete m_trails[i].trail;
	}
}

void CSpeed::Create(EntityHandle numinteractive)
{
	this->num = numinteractive;
	
	std::vector<VertexGroup> & grouplist = entities[this->num]->obj->grouplist;
	
	bool skip = true;
	std::vector<VertexGroup>::const_iterator itr;
	for(itr = grouplist.begin(); itr != grouplist.end(); ++itr) {
		skip = !skip;
		
		if(skip) {
			continue;
		}
		
		float col = 0.05f + (rnd() * 0.05f);
		float size = 1.f + (0.5f * rnd());
		int taille = Random::get(130, 260);
		
		SpeedTrail trail;
		trail.vertexIndex = itr->origin;	
		trail.trail = new Trail(taille, Color4f::gray(col), Color4f::black, size, 0.f);
		
		m_trails.push_back(trail);
	}
}

void CSpeed::Update(float timeDelta)
{
	ARX_UNUSED(timeDelta);
	
	for(size_t i = 0; i < m_trails.size(); i++) {
		Vec3f pos = entities[this->num]->obj->vertexlist3[m_trails[i].vertexIndex].v;
		
		m_trails[i].trail->SetNextPosition(pos);
		m_trails[i].trail->Update(timeDelta);
	}
}

void CSpeed::Render()
{
	for(size_t i = 0; i < m_trails.size(); i++) {
		m_trails[i].trail->Render();
	}
}

CCreateFood::CCreateFood() {
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;
	pPS = new ParticleSystem();
}

CCreateFood::~CCreateFood() {
	delete pPS;
}

void CCreateFood::Create() {
	
	eSrc = player.pos;
	
	pPS->SetPos(eSrc);
	ParticleParams cp = ParticleParams();
	cp.m_nbMax = 350;
	cp.m_life = 800;
	cp.m_lifeRandom = 2000;
	cp.m_pos = Vec3f(100, 200, 100);
	cp.m_direction = Vec3f(0, -10, 0) * 0.1f;
	cp.m_angle = radians(5);
	cp.m_speed = 120;
	cp.m_speedRandom = 84;
	cp.m_gravity = Vec3f(0, -10, 0);
	cp.m_flash = 0;
	cp.m_rotation = 1.0f / (101 - 80);

	cp.m_startSegment.m_size = 8;
	cp.m_startSegment.m_sizeRandom = 8;
	cp.m_startSegment.m_color = Color(105, 105, 20, 145).to<float>();
	cp.m_startSegment.m_colorRandom = Color(50, 50, 0, 10).to<float>();

	cp.m_endSegment.m_size = 6;
	cp.m_endSegment.m_sizeRandom = 4;
	cp.m_endSegment.m_color = Color(20, 20, 5, 0).to<float>();
	cp.m_endSegment.m_colorRandom = Color(40, 40, 0, 0).to<float>();

	cp.m_blendMode = RenderMaterial::Additive;
	cp.m_texture.set("graph/particles/create_food", 0, 100); //5
	cp.m_spawnFlags = PARTICLE_CIRCULAR | PARTICLE_BORDER;
	
	pPS->SetParams(cp);
}

void CCreateFood::Update(float timeDelta)
{
	ulCurrentTime += timeDelta;
	
	eSrc = entities.player()->pos;
	
	if(ulCurrentTime <= ulDuration) {
		unsigned long ulCalc = ulDuration - ulCurrentTime ;
		arx_assert(ulCalc <= LONG_MAX);
		long ff =  static_cast<long>(ulCalc);
		
		if(ff < 1500) {
			pPS->m_parameters.m_spawnFlags = PARTICLE_CIRCULAR;
			pPS->m_parameters.m_gravity = Vec3f_ZERO;
			
			std::list<Particle *>::iterator i;
			
			for(i = pPS->listParticle.begin(); i != pPS->listParticle.end(); ++i) {
				Particle * pP = *i;
				
				if(pP->isAlive()) {
					pP->fColorEnd.a = 0;
						
					if(pP->m_age + ff < pP->m_timeToLive) {
						pP->m_age = pP->m_timeToLive - ff;
					}
				}
			}
		}
	}

	pPS->SetPos(eSrc);
	pPS->Update(timeDelta);
}

void CCreateFood::Render()
{
	pPS->Render();
}

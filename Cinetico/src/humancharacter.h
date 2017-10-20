#ifndef __CINETICO_UI_HUMANCHARACTER_H__
#define __CINETICO_UI_HUMANCHARACTER_H__

#include "character.h"
#include "entity/core/Body.h"
#include "entity/core/BodyPoint.h"

namespace cinetico {

	using namespace cinetico_core;

	class HumanCharacter : public Character {
		Body *m_body;

		void mapBodyPointToWorldPoint(int instId, BodyPoint::BodyPart bodyPoint);
	public:
		HumanCharacter(CineticoUI &cinetico);

		virtual void update();
		virtual void render();
	};

}

#endif
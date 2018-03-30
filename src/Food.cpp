#include "config.h"

#include "Food.h"

Food::Food(Field *field, const Vector2D &pos, real_t value)
	: PositionObject(pos)
	, m_field(field)
	, m_value(value)
{
}

void Food::decay(void)
{
	m_value -= config::FOOD_DECAY_STEP;
}

bool Food::hasDecayed(void)
{
	return m_value <= 0;
}

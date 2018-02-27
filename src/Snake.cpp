#include "config.h"

#include "Field.h"

#include "Snake.h"

Snake::Snake(Field *field)
	: m_field(field), m_mass(1.0f), m_heading(0.0f)
{
	std::shared_ptr<Segment> segment = std::make_shared<Segment>();
	segment->vel = Vector(0.1, 0);
	m_segments.push_back(segment);

	ensureSizeMatchesMass();
}

Snake::Snake(Field *field, const Vector &startPos, float_t start_mass,
		float_t start_heading)
	: m_field(field), m_mass(start_mass), m_heading(start_heading)
{
	// create the first segment manually
	std::shared_ptr<Segment> segment = std::make_shared<Segment>();
	segment->pos = startPos;
	segment->vel = Vector(0.1, 0);
	segment->vel.rotate(m_heading);

	m_segments.push_back(segment);

	// create the other segments
	ensureSizeMatchesMass();
}

void Snake::ensureSizeMatchesMass(void)
{
	std::size_t curLen = m_segments.size();
	std::size_t targetLen = static_cast<std::size_t>(
			pow(m_mass, config::SNAKE_LENGTH_EXPONENT) + 0.5);

	// ensure there are at least 2 segments to define movement direction
	if(targetLen < 2) {
		targetLen = 2;
	}

	if(curLen < targetLen) {
		// segments have to be added:
		// repeat the last segment until the new target length is reached
		std::shared_ptr<Segment> refSegment = m_segments[curLen-1];
		for(std::size_t i = 0; i < (targetLen - curLen); i++) {
			std::shared_ptr<Segment> segment = std::make_shared<Segment>();
			segment->pos = refSegment->pos - refSegment->vel;
			segment->vel = refSegment->vel;

			m_segments.push_back(segment);

			refSegment = segment;
		}
	} else if(curLen > targetLen) {
		// segments must be removed
		m_segments.resize(targetLen);
	}

	// update segment radius
	m_segmentRadius = std::sqrt(m_mass) / 2;
}

Vector Snake::calculateDeltaV(
		const std::shared_ptr<Segment> &seg1,
		const std::shared_ptr<Segment> &seg2)
{
	float_t dist = seg1->pos.distanceTo(seg2->pos);

	if(dist == 0) {
		return Vector(0, 0);
	}

	float_t distErr = dist - config::SNAKE_BASE_DISTANCE;

	Vector deltaV(seg2->pos - seg1->pos);
	deltaV.normalizeToLength(distErr);
	return deltaV * config::SNAKE_SPRING_CONSTANT;
}

float_t Snake::maxRotationPerStep(void)
{
	// TODO: make this better?
	return 10.0 / (m_segmentRadius/10.0 + 1);
}

void Snake::consume(const std::shared_ptr<Food>& food)
{
	m_mass += food->getValue();
	ensureSizeMatchesMass();
}

std::size_t Snake::move(float_t targetAngle, bool boost)
{
	// increase step size while boosting
	float_t speed_scale = 1.0f;
	if(boost) {
		speed_scale = config::SNAKE_BOOST_SPEEDUP;
	}

	// Step 0: unwrap all coordinates
	for(std::size_t i = 0; i < m_segments.size(); i++) {
		auto &seg = m_segments[i];
		auto &ref = (i>0) ? m_segments[i-1] : m_segments[0];

		seg->pos = m_field->unwrapCoords(seg->pos, ref->pos);
	}

	// Step 1: move all segments except the head forward by their velocity

	for(std::size_t i = 1; i < m_segments.size(); i++) {
		auto &seg = m_segments[i];
		seg->pos += seg->vel;
	}

	// Step 2: calculate new head position

	// calculate delta angle
	float_t deltaAngle = targetAngle - m_heading;

	// normalize delta angle
	if(deltaAngle > 180) {
		deltaAngle -= 360;
	} else if(deltaAngle < -180) {
		deltaAngle += 360;
	}

	// limit rotation rate
	float_t maxDelta = maxRotationPerStep();
	if(deltaAngle > maxDelta) {
		deltaAngle = maxDelta;
	} else if(deltaAngle < -maxDelta) {
		deltaAngle = -maxDelta;
	}

	// calculate new head position
	m_heading += deltaAngle;
	Vector movementVector(config::SNAKE_DISTANCE_PER_STEP * speed_scale, 0.0f);
	movementVector.rotate(m_heading * M_PI / 180);

	m_segments[0]->pos += movementVector;
	m_segments[0]->vel = movementVector;

	// Step 3: apply friction
	// reduces the velocity by a configurable factor
	for(std::size_t i = 0; i < m_segments.size(); i++) {
		m_segments[i]->vel *= config::SNAKE_FRICTION_FACTOR;
	}

	// Step 4: apply spring-mass network
	// Simulates springs between the Snake segments, which cause the Snake to stay together.
	// All segments except the head are influenced by this.
	for(std::size_t i = 1; i < m_segments.size(); i++) {
		Vector deltaV(0, 0);

		deltaV += calculateDeltaV(m_segments[i], m_segments[i-1]);

		if(i < m_segments.size()-1) {
			deltaV += calculateDeltaV(m_segments[i], m_segments[i+1]);
		}

		m_segments[i]->vel += deltaV;
	}

	// Step N+1: wrap all coordinates
	for(std::size_t i = 0; i < m_segments.size(); i++) {
		auto &seg = m_segments[i];
		seg->pos = m_field->wrapCoords(seg->pos);
	}

	// normalize heading
	if(m_heading > 180) {
		m_heading -= 360;
	} else if(m_heading < -180) {
		m_heading += 360;
	}

	return m_segments.size(); // == number of new segments at head
}

const Snake::SegmentList& Snake::getSegments(void) const
{
	return m_segments;
}

const Vector& Snake::getHeadPosition(void) const
{
	return m_segments[0]->pos;
}

float_t Snake::getSegmentRadius(void) const
{
	return m_segmentRadius;
}

bool Snake::canConsume(const std::shared_ptr<Food> &food)
{
	const Vector &headPos = m_segments[0]->pos;
	const Vector &foodPos = food->getPosition();

	float_t hx = headPos.x();
	float_t hy = headPos.y();

	Vector unwrappedFoodPos = m_field->unwrapCoords(foodPos, headPos);
	float_t fx = unwrappedFoodPos.x();
	float_t fy = unwrappedFoodPos.y();

	// quick range check
	float_t maxRange = m_segmentRadius * config::SNAKE_CONSUME_RANGE;
	if(fx > (hx + maxRange) ||
			fx < (hx - maxRange) ||
			fy > (hy + maxRange) ||
			fy < (hy - maxRange)) {
		return false;
	}

	// thorough range check
	return headPos.distanceTo(unwrappedFoodPos) < maxRange;
}

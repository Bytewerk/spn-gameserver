/*!
 * \file
 *
 * \brief Game constants configuration file.
 * \details
 * This file defines global, static constants used by different parts of the
 * game in a common place.
 */
#pragma once

#include "types.h"

namespace config {

	// Field size
	static const float_t     FIELD_SIZE_X            = 1024;
	static const float_t     FIELD_SIZE_Y            = 1024;

	// Items of static food on field
	static const std::size_t FIELD_STATIC_FOOD       = 5000;

	// Radius around the head of a snake in which food is eaten
	static const float_t     SNAKE_EATING_RADIUS     = 2.0;

	// Factor by that the Snake’s speed increases while boosting
	static const float_t     SNAKE_BOOST_SPEEDUP     = 3.0;

	// Distance per normal movement step
	static const float_t     SNAKE_DISTANCE_PER_STEP = 1.0;

	// A factor to apply to the velocity of each segment each frame
	static const float_t     SNAKE_FRICTION_FACTOR   = 0.95;

	// Spring constant of the springs between the segments
	static const float_t     SNAKE_SPRING_CONSTANT   = 0.500;

	// Base value for the distance between segments
	static const float_t     SNAKE_BASE_DISTANCE     = 0.000;

	static const float_t     SNAKE_LENGTH_EXPONENT   = 0.8;

	// Distance multiplier for the Snake’s consume range. This is multiplied with
	// the Snake’s segment radius.
	static const float_t     SNAKE_CONSUME_RANGE     = 1.0;

	// Food particle size log-normal distribution parameters
	static const float_t     FOOD_SIZE_MEAN          = 3.5;
	static const float_t     FOOD_SIZE_STDDEV        = 2.0;

	// Food decay value per frame
	static const float_t     FOOD_DECAY_STEP         = 0.010;
}

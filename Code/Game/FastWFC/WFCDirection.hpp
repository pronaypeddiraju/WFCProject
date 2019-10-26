#pragma once

//------------------------------------------------------------------------------------------------------------------------------
//Direction is represented as uint between 0 to 3
//x and y value of direction is retrieved from these tables
constexpr int directions_x[4] = { 0, -1, 1, 0 };
constexpr int directions_y[4] = { -1, 0, 0, 1 };

//------------------------------------------------------------------------------------------------------------------------------
//Returns the opposite direction of 'direction' argument
constexpr unsigned int GetOppositeDirection(unsigned int direction) noexcept
{
	return 3 - direction;
}
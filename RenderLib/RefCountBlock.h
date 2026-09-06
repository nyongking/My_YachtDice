#pragma once

template<typename T>
struct RefCountBlock
{
	unsigned int				refcount = 1;
	T							member;
};
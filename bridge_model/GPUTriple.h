#pragma once
class GPUTriple
{
protected:
	static int sm_current;
	
public:
	static void Swap();
	static void Lock();
};


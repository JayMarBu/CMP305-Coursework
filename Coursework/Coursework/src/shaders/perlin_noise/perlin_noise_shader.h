#pragma once
#include "BaseShader.h"

class PerlinNoiseShader : public BaseShader
{
public:
private:
	void initShader(const wchar_t*, const wchar_t*) override;
};
#pragma once
#include<unordered_map>
namespace tEngine {

	enum class SV {
		_MATRIX_V,
		_MATRIX_P,
		_MATRIX_VP,
		_INV_MATRIX_VP,
		_MATRIX_M
	};
	static std::string ShaderString(SV v) {
		switch (v) {
		case SV::_MATRIX_V:return  "_MATRIX_V";
		case SV::_MATRIX_P:return "_MATRIX_P";
		case SV::_MATRIX_VP:return "_MATRIX_VP";
		case SV::_MATRIX_M:return "_MATRIX_M";
		case SV::_INV_MATRIX_VP:return "_INV_MATRIX_VP";
			
		}
		assert(false);
		return "";
	}
	
}
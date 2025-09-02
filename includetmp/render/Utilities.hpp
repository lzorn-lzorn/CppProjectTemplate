#pragma once

#include <vector>
#include <functional>
#include <algorithm>
namespace Render{

template <typename T, typename U>
void RemoveUnsupportedElems(
	std::vector<T>& elems, 
	const std::vector<U>& support,
	std::function<bool(const T&, const U&)> IsEqual
){
	int i = 0;
    while (i < elems.size()) {
        if (std::find_if(
				support.begin(), 
				support.end(), 
				[&](const U& e) {
					return IsEqual(elems[i], e);
				}) == support.end()) 
		{
            elems.erase(elems.begin() + i);
        } else {
            i ++;
        }
    }

}


}
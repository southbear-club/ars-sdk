/**
 * Copyright © 2021 <wotsen>.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the “Software”), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * @file iniparser.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <string>
using std::string;

namespace ars {
    
namespace sdk {

#define ARS_DEFAULT_INI_COMMENT "#"
#define ARS_DEFAULT_INI_DELIM   "="

// fwd
class IniNode;

class IniParser {
public:
    IniParser();
    ~IniParser();

    int LoadFromFile(const char* filepath);
    int LoadFromMem(const char* data);
    int Unload();
    int Reload();

    string DumpString();
    int Save();
    int SaveAs(const char* filepath);

    string GetValue(const string& key, const string& section = "");
    void   SetValue(const string& key, const string& value, const string& section = "");

    // T = [bool, int, float]
    template<typename T>
    T Get(const string& key, const string& section = "", T defvalue = 0);

    // T = [bool, int, float]
    template<typename T>
    void Set(const string& key, const T& value, const string& section = "");

protected:
    void DumpString(IniNode* pNode, string& str);

public:
    string  _comment;
    string  _delim;
    string  _filepath;
private:
    IniNode* root_;
};
    
} // namespace sdk

} // namespace ars

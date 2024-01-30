#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>


int main()
{
    std::string text = "textwrappingisfun";
    char * p = "abcde";
    printf("%lu --- %lu\n", strlen(p), sizeof("abcde"));
    std::vector<std::string> out;
    for(uint32_t idx = 0; idx < text.length(); idx+=4)
    {
        printf("idx = %d\n", idx);
        out.push_back(text.substr(idx, 4));

    }

    for (std::string val : out) {
        printf("%s\n", val.c_str());
    }

    return (0);
}

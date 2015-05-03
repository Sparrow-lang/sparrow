#include <cmph.h>
#include <stdio.h>
#include <string.h>

#include <vector>
#include <string>
#include <iostream>

using namespace std;

static const unsigned maxKeys = -1;



void checkHashIsOk(cmph_t* hash, char** keysArray, size_t numKeys)
{
    vector<bool> usedHashes(numKeys);

    for ( size_t i=0; i<numKeys; ++i )
    {
        const char* key = keysArray[i];
        unsigned int id = cmph_search(hash, key, (cmph_uint32)strlen(key));

        // fprintf(stderr, "Key: %s -> %u\n", key, id);

        if ( usedHashes[id] )
            cout << "FAILURE: Hash " << id << " was already used\n";
        if ( id >= numKeys )
            cout << "FAILURE: Hash " << id << " is too big (max: " << numKeys << ")\n";

        usedHashes[id] = true;
    }
}

 // Create minimal perfect hash function from in-disk keys using CHD algorithm
int main(int argc, char **argv)
{
    int dummyArg = 0;
    int maxKeys = argc > 2 ? atoi(argv[2]) : 1;
    int testRepCount = argc > 3 ? atoi(argv[3]) : 1;
    int totalRepCount = argc > 4 ? atoi(argv[4]) : 1;

    vector<string> keys;

    // Read the content of the file
    FILE* f = fopen("words-full", "r");
    if ( !f )
    {
        fprintf(stderr, "File \"words\" not found\n");
        exit(1);
    }
    while ( !feof(f) && keys.size() < maxKeys )
    {
        char buf[513];
        const char* s = fgets(buf, 512, f);
        if ( s )
        {
            string str = s;
            str.resize(str.size()-1);   // Remove newline
            keys.push_back(str);
        }
    }
    fclose(f);

    // for ( size_t i=0; i<keys.size(); ++i )
    // {
    //     cout << "Key: " << keys[i] << "\n";
    // }

    // Transform the keys into C strings
    char** keysArray = new char*[keys.size()];
    for ( size_t i=0; i<keys.size(); ++i )
    {
        keysArray[i] = &keys[i][0];
    }


    for ( int i=0; i<totalRepCount; ++i )
    {
        cmph_io_adapter_t* source = cmph_io_vector_adapter(keysArray, keys.size());
        cmph_config_t* config = cmph_config_new(source);
        cmph_config_set_algo(config, CMPH_CHD);
        cmph_config_set_graphsize(config, 0.99);
        cmph_t* hash = cmph_new(config);
        cmph_config_destroy(config);
        cmph_io_nlfile_adapter_destroy(source);

        for ( int j=0; j<testRepCount; ++j )
        {
            checkHashIsOk(hash, keysArray, keys.size());
        }

        cmph_destroy(hash);
    }
    
    return 0;
}

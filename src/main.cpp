#include "json_reader.h"

using namespace std;

int main() {
    Transport::TransportCatalogue catalogue;
    Transport::renderer::MapRenderer renderer;
    {
        Transport::input::JsonReader reader;
        reader.ParseCommands(cin);
        reader.ApplyCommands(catalogue, renderer);
    }
}
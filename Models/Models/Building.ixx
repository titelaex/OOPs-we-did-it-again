export module Building;

#ifdef BUILDING_DLL
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

export namespace Models
{
    export class DLL_API Building
    {
    public:
        Building() = default;
        ~Building() = default;
    };
}
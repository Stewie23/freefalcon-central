#include "stdhdr.h"
#include "acdef.h"
#include "wpndef.h"
#include "helodef.h"
#include "gnddef.h"
#include "simfile.h"
#include "soundfx.h"

#define SIM_VEHICLE_DEFINITION_FILE    "sim\\vehdef\\vehicle.lst"

#ifdef USE_SH_POOLS
extern MEM_POOL gReadInMemPool;
#endif

SimMoverDefinition** moverDefinitionData = NULL;
int NumSimMoverDefinitions = 0;

static SimlibFileClass* OpenRequiredSimFile(char* fileName)
{
    SimlibFileClass* file = SimlibFileClass::Open(fileName, SIMLIB_READ);

    if (file == NULL)
    {
        char alternateName[_MAX_PATH];
        strncpy(alternateName, fileName, sizeof(alternateName) - 1);
        alternateName[sizeof(alternateName) - 1] = '\0';
        _strlwr(alternateName);

        if (strcmp(alternateName, fileName))
            file = SimlibFileClass::Open(alternateName, SIMLIB_READ);
    }

    if (file == NULL)
    {
        char alternateName[_MAX_PATH];
        strncpy(alternateName, fileName, sizeof(alternateName) - 1);
        alternateName[sizeof(alternateName) - 1] = '\0';

        for (char *ptr = alternateName; *ptr; ptr++)
        {
            if (*ptr == '\\')
                *ptr = '/';
        }

        if (strcmp(alternateName, fileName))
            file = SimlibFileClass::Open(alternateName, SIMLIB_READ);
    }

    if (file == NULL)
    {
        char message[_MAX_PATH + 160];
        sprintf(message, "Failed to open sim mover definition file:\n\n%s\n\nError: %d\n%s  %s", fileName, __LINE__, __FILE__, __DATE__);
        MessageBox(NULL, message, "FreeFalcon startup error", MB_OK bitor MB_ICONERROR);
        exit(-1);
    }

    return file;
}

SimMoverDefinition::SimMoverDefinition(void)
{
    numSensors = 0;
    sensorData = NULL;
}

SimMoverDefinition::~SimMoverDefinition(void)
{
}

void SimMoverDefinition::ReadSimMoverDefinitionData(void)
{
    int i;
    SimlibFileClass* vehList;
    int vehicleType;

    vehList = OpenRequiredSimFile(SIM_VEHICLE_DEFINITION_FILE);

    NumSimMoverDefinitions = atoi(vehList->GetNext());
#ifdef USE_SH_POOLS
    moverDefinitionData = (SimMoverDefinition **)MemAllocPtr(gReadInMemPool, sizeof(SimMoverDefinition*)*NumSimMoverDefinitions, 0);
#else
    moverDefinitionData = new SimMoverDefinition*[NumSimMoverDefinitions];
#endif

    for (i = 0; i < NumSimMoverDefinitions; i++)
    {
        vehicleType = atoi(vehList->GetNext());

        switch (vehicleType)
        {
            case Aircraft:
                moverDefinitionData[i] = new SimACDefinition(vehList->GetNext());
                break;

            case Ground:
                moverDefinitionData[i] = new SimGroundDefinition(vehList->GetNext());
                break;

            case Helicopter:
                moverDefinitionData[i] = new SimHeloDefinition(vehList->GetNext());
                break;

            case Weapon:
                moverDefinitionData[i] = new SimWpnDefinition(vehList->GetNext());
                break;

            case Sea:
                vehList->GetNext();
                moverDefinitionData[i] = new SimMoverDefinition;
                break;

            default:
                vehList->GetNext();
                moverDefinitionData[i] = new SimMoverDefinition;
                break;
        }
    }

    vehList->Close();
    delete vehList;
}

void SimMoverDefinition::FreeSimMoverDefinitionData(void)
{
    int i;

    for (i = 0; i < NumSimMoverDefinitions; i++)
    {
        delete moverDefinitionData[i];
    }

#ifdef USE_SH_POOLS
    MemFreePtr(moverDefinitionData);
#else
    delete [] moverDefinitionData;
#endif
}

SimACDefinition::SimACDefinition(char* fileName)
{
    int i;
    SimlibFileClass* acFile;

    acFile = OpenRequiredSimFile(fileName);

    // What type of combat does it do?
    combatClass = (CombatClass)atoi(acFile->GetNext());

    airframeIndex = atoi(acFile->GetNext());
    numPlayerSensors  =  atoi(acFile->GetNext());

#ifdef USE_SH_POOLS
    playerSensorData = (int *)MemAllocPtr(gReadInMemPool, sizeof(int) * numPlayerSensors * 2, 0);
#else
    playerSensorData = new int [numPlayerSensors * 2];
#endif

    for (i = 0; i < numPlayerSensors; i++)
    {
        playerSensorData[i * 2]  =  atoi(acFile->GetNext());
        playerSensorData[i * 2 + 1]  =  atoi(acFile->GetNext());
    }

    numSensors  =  atoi(acFile->GetNext());

#ifdef USE_SH_POOLS
    sensorData = (int *)MemAllocPtr(gReadInMemPool, sizeof(int) * numSensors * 2, 0);
#else
    sensorData = new int [numSensors * 2];
#endif

    for (i = 0; i < numSensors; i++)
    {
        sensorData[i * 2]  =  atoi(acFile->GetNext());
        sensorData[i * 2 + 1]  =  atoi(acFile->GetNext());
    }

    acFile->Close();
    delete acFile;
}

SimACDefinition::~SimACDefinition(void)
{
#ifdef USE_SH_POOLS
    MemFreePtr(sensorData);
    MemFreePtr(playerSensorData);
#else
    delete [] sensorData;
    delete [] playerSensorData;
#endif
}

SimWpnDefinition::SimWpnDefinition(char* fileName)
{
    SimlibFileClass* wpnFile;

    wpnFile = OpenRequiredSimFile(fileName);

    flags = atoi(wpnFile->GetNext());
    cd  = (float)atof(wpnFile->GetNext());
    weight = (float)atof(wpnFile->GetNext());
    area  = (float)atof(wpnFile->GetNext());
    xEjection  = (float)atof(wpnFile->GetNext());
    yEjection  = (float)atof(wpnFile->GetNext());
    zEjection  = (float)atof(wpnFile->GetNext());
    strcpy(mnemonic, wpnFile->GetNext());
    weaponClass  = atoi(wpnFile->GetNext());
    domain  = atoi(wpnFile->GetNext());
    weaponType  = atoi(wpnFile->GetNext());
    dataIdx  = atoi(wpnFile->GetNext());

    wpnFile->Close();
    delete wpnFile;
}

SimWpnDefinition::~SimWpnDefinition(void)
{
}

SimHeloDefinition::SimHeloDefinition(char* fileName)
{
    int i;
    SimlibFileClass* heloFile;

    heloFile = OpenRequiredSimFile(fileName);

    airframeIndex = atoi(heloFile->GetNext());
    numSensors  =  atoi(heloFile->GetNext());

#ifdef USE_SH_POOLS
    sensorData = (int *)MemAllocPtr(gReadInMemPool, sizeof(int) * numSensors * 2, 0);
#else
    sensorData = new int [numSensors * 2];
#endif

    for (i = 0; i < numSensors; i++)
    {
        sensorData[i * 2]  =  atoi(heloFile->GetNext());
        sensorData[i * 2 + 1]  =  atoi(heloFile->GetNext());
    }

    heloFile->Close();
    delete heloFile;
}

SimHeloDefinition::~SimHeloDefinition(void)
{
#ifdef USE_SH_POOLS
    MemFreePtr(sensorData);
#else
    delete [] sensorData;
#endif
}

SimGroundDefinition::SimGroundDefinition(char* fileName)
{
    int i;
    SimlibFileClass* gndFile;

    gndFile = OpenRequiredSimFile(fileName);

    numSensors  =  atoi(gndFile->GetNext());

#ifdef USE_SH_POOLS
    sensorData = (int *)MemAllocPtr(gReadInMemPool, sizeof(int) * numSensors * 2, 0);
#else
    sensorData = new int [numSensors * 2];
#endif

    for (i = 0; i < numSensors; i++)
    {
        sensorData[i * 2]  =  atoi(gndFile->GetNext());
        sensorData[i * 2 + 1]  =  atoi(gndFile->GetNext());
    }

    gndFile->Close();
    delete gndFile;
}

SimGroundDefinition::~SimGroundDefinition(void)
{
#ifdef USE_SH_POOLS
    MemFreePtr(sensorData);
#else
    delete [] sensorData;
#endif
}

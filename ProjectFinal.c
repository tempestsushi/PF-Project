#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

typedef struct CropMinerals 
{
    char crop_name[20];
    float salinity_min, salinity_max;
    float nitrogen_min, nitrogen_max;
    float potassium_min, potassium_max;
    float calcium_min, calcium_max;
    float magnesium_min, magnesium_max;
    float water_pH_min, water_pH_max;
    float water_table_min, water_table_max;
} CropData;

CropData crops[] = 
{
    {"Wheat", 0, 4, 1.5, 2.5, 0.5, 1.0, 1.0, 1.5, 0.4, 0.8, 6.5, 7.5, 3.0, 10.0},
    {"Cotton", 0, 5, 1.2, 2.0, 0.4, 0.8, 1.5, 2.0, 0.3, 0.7, 7.0, 8.0, 3.0, 5.0},
    {"Rice", 0, 5, 1.0, 1.8, 0.4, 0.9, 0.8, 1.2, 0.3, 0.6, 6.0, 7.0, 1.5, 5.0},
    {"Sugarcane", 0, 4, 1.2, 2.5, 0.6, 1.0, 1.2, 2.0, 0.3, 0.8, 7.0, 8.0, 3.0, 6.0}
};

typedef struct Regions 
{
    char crop_name[20];
    char regions[3][20];
} RegionValidity;

RegionValidity Regions[] = 
{
    {"Wheat", {"Punjab", "Sindh", "KPK"}},
    {"Cotton", {"Sindh", "Punjab", "KPK"}},
    {"Rice", {"Punjab", "Sindh", "KPK"}},
    {"Sugarcane", {"Punjab", "Sindh", "KPK"}}
};

typedef struct FertilizerPerAcre 
{
    float gypsum_per_acre;
    float urea_per_acre;
} Fertilizer;

typedef struct 
{
    int day, month, year;
} Date;

int ValidateRegion(char *crop_name, char *region, RegionValidity Regions[], int crop_count);
int ValidateCrop(char *crop_name, CropData crops[], int crop_count);
void AnalysisofSoilWater(CropData crop, float *values, char *region, char *season, Fertilizer fertilizerConfig, FILE *file, float landarea);
void EstimateMinerals(int land_area, float salinity, float nitrogen, FILE *file);
void savereportfile(char *region, char *season, CropData crop, float *values, Date date, FILE *file, int recovery_months, int issues);

int main() {
    char region[20], season[20], cropname[20];
    int regionchoice, seasonchoice;
    float landarea;

    int cropcount = sizeof(crops) / sizeof(crops[0]);
    int regionscount = sizeof(Regions) / sizeof(Regions[0]);

    Date date;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    date.day = tm.tm_mday;
    date.month = tm.tm_mon + 1;
    date.year = tm.tm_year + 1900;

    printf("Select the region:\n1. Sindh\n2. Punjab\n3. Balochistan\n4. KPK\n");
    printf("Enter: ");
    scanf("%d", &regionchoice);
    getchar();

    switch (regionchoice) 
    {
        case 1: strcpy(region, "Sindh"); break;
        case 2: strcpy(region, "Punjab"); break;
        case 3: strcpy(region, "Balochistan"); break;
        case 4: strcpy(region, "KPK"); break;
        default:
            printf("Invalid region choice. Exiting program.\n");
            return 1;
    }

    printf("Select the season:\n1. Winter (December to February)\n2. Spring (March to May)\n3. Summer (June to August)\n4. Autumn (September to November)\n");
    printf("Enter: ");
    scanf("%d", &seasonchoice);
    getchar();

    switch (seasonchoice) 
    {
        case 1: strcpy(season, "Winter"); break;
        case 2: strcpy(season, "Spring"); break;
        case 3: strcpy(season, "Summer"); break;
        case 4: strcpy(season, "Autumn"); break;
        default:
            printf("Invalid season choice. Exiting program.\n");
            return 1;
    }

    printf("Enter the area of land being used for farming (in acres): ");
    scanf("%f", &landarea);
    getchar();

    printf("Enter the crop (Wheat, Cotton, Rice, Sugarcane): ");
    fgets(cropname, sizeof(cropname), stdin);
    cropname[strcspn(cropname, "\n")] = '\0';

    CropData selectedcrop;
    if (!ValidateCrop(cropname, crops, cropcount)) 
    {
        printf("Error: The crop '%s' is invalid.\n", cropname);
        return 1;
    }

    for (int i = 0; i < cropcount; i++) 
    {
        if (strcmp(cropname, crops[i].crop_name) == 0) 
        {
            selectedcrop = crops[i];
            break;
        }
    }

    if (ValidateRegion(cropname, region, Regions, regionscount)) 
    {
        printf("The crop '%s' can grow in region '%s'.\n", cropname, region);
    } 
    else 
    {
        printf("Error: The crop '%s' does not grow in region '%s'.\n", cropname, region);
        return 1;
    }

    float values[7];
    printf("Enter Soil Salinity (dS/m): ");
    scanf("%f", &values[0]);
    printf("Enter Nitrogen (%%): ");
    scanf("%f", &values[1]);
    printf("Enter Potassium (%%): ");
    scanf("%f", &values[2]);
    printf("Enter Calcium (%%): ");
    scanf("%f", &values[3]);
    printf("Enter Magnesium (%%): ");
    scanf("%f", &values[4]);
    printf("Enter Water pH: ");
    scanf("%f", &values[5]);
    printf("Enter Water Table Depth (feet): ");
    scanf("%f", &values[6]);

    int recoverymonths = 0;
    int issues = 0;

    Fertilizer fertilizerConfig = {500.0, 50.0}; 

    FILE *file = fopen("report.txt", "w");
    if (file == NULL) 
    {
        printf("Unable to open the file for writing.\n");
        return 1;
    }

    AnalysisofSoilWater(selectedcrop, values, region, season, fertilizerConfig, file, landarea);
    // Estimate Minerals will be passed throught Analyze Functions
    savereportfile(region, season, selectedcrop, values, date, file, recoverymonths, issues);
    fclose(file);

    return 0;
}

// Validity of Region and Crop Function: Crop validity is basically the crop you entered is valid to the pre-defined or not.
//                                       Region validity is if the crop even grows in that region:
//                                           {"Wheat", {"Punjab", "Sindh", "KPK"}},
//                                           {"Cotton", {"Sindh", "Punjab", "KPK"}},
//                                           {"Rice", {"Punjab", "Sindh", "KPK"}},
//                                           {"Sugarcane", {"Punjab", "Sindh", "KPK"}}
//                                       Wheat only grows in Punjab Sindh and KPK etc.

//SoilWater Analysis Function : take the input values and comparing with the pre-defined values and creating a algorithm:
// this alogrithm finds how bad the condition of the soil is like if the compared values are all high the soil condition is poor meaning unfertile.
// 1 = fertile 2 = sub-fertile 3 = unfertile

//Estimae Minerals Functions : After inputting % of each minerals, how much 
//    "Apply gypsum to reduce salinity", "Add urea fertilizer for nitrogen boost", "Add potassium-based fertilizers",
//    "Add lime to improve calcium levels", "Add magnesium sulfate to correct magnesium deficiency", 
//    "Adjust soil pH using sulfur-based additives", "Improve drainage or irrigation practices"
// per Acre is supposed to be used.








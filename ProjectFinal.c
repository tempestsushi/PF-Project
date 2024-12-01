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
void EstimateMinerals(int land_area, float salinity, float nitrogen, float potassium, float calcium, float magnesium, 
                      float *gypsum_required, float *urea_required, float *potash_required, float *lime_required, 
                      float *sulfur_required, float *magnesium_sulfate_required, float *calcium_sulfate_required);
void AnalysisofSoilAndWater(CropData crop, float *values, char *region, char *season, Fertilizer fertilizerConfig, FILE *file, int landarea, Date date);
void SaveReportFile(FILE *file, CropData crop, const char *region, const char *season, float *values, int land_area, 
                    float gypsum_required, float urea_required, float potash_required, float lime_required, 
                    float sulfur_required, float magnesium_sulfate_required, float calcium_sulfate_required, Date date, int recovery_months);

int main() 
{
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

    AnalysisofSoilAndWater(selectedcrop, values, region, season, fertilizerConfig, file, landarea, date);

    return 0;
}

// ValidateCrop inputted Crop
int ValidateCrop(char *crop_name, CropData crops[], int crop_count) 
{
    for (int i = 0; i < crop_count; i++) {
        if (strcmp(crop_name, crops[i].crop_name) == 0) 
        {
            return 1;
        }
    }
    return 0;
}

// Validate if the crop grows in that region
int ValidateRegion(char *crop_name, char *region, RegionValidity Regions[], int region_count) 
{
    for (int i = 0; i < region_count; i++) 
    {
        if (strcmp(crop_name, Regions[i].crop_name) == 0) 
        {
            for (int j = 0; j < 3; j++) {
                if (strcmp(region, Regions[i].regions[j]) == 0) 
                {
                    return 1; 
                }
            }
        }
    }
    return 0; 
}

// Function to analyze soil and water conditions
void AnalysisofSoilAndWater(CropData crop, float *values, char *region, char *season, Fertilizer fertilizerConfig, FILE *file, int landarea, Date date) 
{
    printf("\nSoil and Water Analysis for Crop: %s\nRegion: %s | Season: %s\n", crop.crop_name, region, season);

    const char *parameters[] = {"Soil Salinity", "Nitrogen", "Potassium", "Calcium", "Magnesium", "Water pH", "Water Table Depth"};
    float ranges[][2] = 
    {
        {crop.salinity_min, crop.salinity_max}, {crop.nitrogen_min, crop.nitrogen_max},
        {crop.potassium_min, crop.potassium_max},
        {crop.calcium_min, crop.calcium_max}, {crop.magnesium_min, crop.magnesium_max},
        {crop.water_pH_min, crop.water_pH_max},
        {crop.water_table_min, crop.water_table_max}
    };

    const char *units[] = {"dS/m", "%", "%", "%", "%", "", "feet"};
    
    // Separate actions for low and high values
    const char *lowActions[] = 
    {
        "Apply gypsum to reduce salinity", 
        "Add urea fertilizer for nitrogen boost", 
        "Add potassium-based fertilizers",
        "Add lime to improve calcium levels", 
        "Add magnesium sulfate to correct magnesium deficiency", 
        "Adjust soil pH using sulfur-based additives", 
        "Improve drainage or irrigation practices"
    };

    const char *highActions[] = 
    {
        "Reduce irrigation or apply leaching to lower salinity", 
        "Reduce nitrogen input or use nitrification inhibitors", 
        "Reduce potassium fertilizers",
        "Reduce calcium-based fertilizers", 
        "Reduce magnesium sulfate use", 
        "Use sulfur to lower water pH", 
        "Improve irrigation system to maintain proper water levels"
    };

    int recovery_months = 0;
    int issues = 0;
    int overall_health = 1;

    for (int i = 0; i < 7; i++) 
    {
        if (values[i] < ranges[i][0]) 
        {
            printf("- %s: %.2f %s - Low (%s)\n", parameters[i], values[i], units[i], lowActions[i]);
            recovery_months = 6;
            issues++;
            overall_health = 3;
        } 
        else if (values[i] > ranges[i][1]) 
        {
            printf("- %s: %.2f %s - High (%s)\n", parameters[i], values[i], units[i], highActions[i]);
            recovery_months = 6;
            issues++;
            overall_health = 2;
        } 
        else 
        {
            printf("- %s: %.2f %s - Optimal\n", parameters[i], values[i], units[i]);
        }
    }

    if (overall_health == 3) 
    {
        recovery_months = 12;
    } 
    else if (overall_health == 2) 
    {
        recovery_months = 6;
    }

    printf("\nRecovery Time: %d months\n", recovery_months);

    // Estimate materials
    float gypsum_required, urea_required, potash_required, lime_required, sulfur_required, magnesium_sulfate_required, calcium_sulfate_required;
    EstimateMinerals(landarea, values[0], values[1], values[2], values[3], values[4], 
                     &gypsum_required, &urea_required, &potash_required, &lime_required, 
                     &sulfur_required, &magnesium_sulfate_required, &calcium_sulfate_required);
    
    // Save to file
    SaveReportFile(file, crop, region, season, values, landarea, gypsum_required, urea_required, potash_required, 
               lime_required, sulfur_required, magnesium_sulfate_required, calcium_sulfate_required, date, recovery_months);
}

void EstimateMinerals(int land_area, float salinity, float nitrogen, float potassium, float calcium, float magnesium, 
                      float *gypsum_required, float *urea_required, float *potash_required, float *lime_required, 
                      float *sulfur_required, float *magnesium_sulfate_required, float *calcium_sulfate_required) 
{
    // Handle Salinity
    *gypsum_required = 0;
    if (salinity > 4.0)
    {
        *gypsum_required = land_area * 2;
    } 
    else if (salinity < 1.0) 
    {
        *gypsum_required = land_area * 0.5;
    }

    // Handle Nitrogen
    *urea_required = 0;
    if (nitrogen < 1.5) 
    {
        *urea_required = land_area * 1.5;
    }

    // Handle Potassium
    *potash_required = 0;
    if (potassium < 0.6) 
    {
        *potash_required = land_area * 1.0;
    }

    // Handle Calcium
    *lime_required = 0;
    if (calcium <= 0.8) 
    {   
        *lime_required = land_area * 1.5;
    }

    *sulfur_required = 0;
    if (calcium > 1.5) 
    {
        *sulfur_required = land_area * 0.5;
    }

    // Handle Magnesium
    *magnesium_sulfate_required = 0;
    *calcium_sulfate_required = 0;
    if (magnesium < 0.4) 
    {
        *magnesium_sulfate_required = land_area * 0.5;
    }
    else if (magnesium > 0.8) 
    {
        *calcium_sulfate_required = land_area * 0.3;
    }

    if (*gypsum_required == 0) 
        *gypsum_required = 0.0;
    if (*urea_required == 0) 
        *urea_required = 0.0;
    if (*potash_required == 0) 
        *potash_required = 0.0;
    if (*lime_required == 0) 
        *lime_required = 0.0;
    if (*sulfur_required == 0) 
        *sulfur_required = 0.0;
    if (*magnesium_sulfate_required == 0) 
        *magnesium_sulfate_required = 0.0;
    if (*calcium_sulfate_required == 0) 
        *calcium_sulfate_required = 0.0;

    printf("Details saved to the report file.\n");
}


void SaveReportFile(FILE *file, CropData crop, const char *region, const char *season, float *values, int land_area, 
                    float gypsum_required, float urea_required, float potash_required, float lime_required, 
                    float sulfur_required, float magnesium_sulfate_required, float calcium_sulfate_required, Date date, int recovery_months)  
{
    const char *parameters[] = {"Soil Salinity", "Nitrogen", "Potassium", "Calcium", "Magnesium", "Water pH", "Water Table Depth"};
    const char *units[] = {"dS/m", "%", "%", "%", "%", "", "feet"};
    
    fprintf(file, "Soil and Water Analysis Report\n");
    fprintf(file, "---------------------------------\n");
    fprintf(file, "Date: %02d/%02d/%04d\n", date.day, date.month, date.year);
    fprintf(file, "Crop: %s\n", crop.crop_name);
    fprintf(file, "Region: %s\n", region);
    fprintf(file, "Season: %s\n", season);
    fprintf(file, "Land Area: %d acres\n", land_area);
    fprintf(file, "\nSoil and Water Parameters:\n");
    fprintf(file, "---------------------------------\n");
    
    // Log All parameters
    for (int i = 0; i < 7; i++) 
    {
        fprintf(file, "- %s: %.2f %s\n", parameters[i], values[i], units[i]);
    }

    fprintf(file, "\nRecommendations and Material Estimates:\n");
    fprintf(file, "---------------------------------\n");
    
    // Log material estimates
    if (gypsum_required > 0) 
        fprintf(file, "- Gypsum required: %.2f tons\n", gypsum_required);
    if (urea_required > 0) 
        fprintf(file, "- Urea required: %.2f tons\n", urea_required);
    if (potash_required > 0) 
        fprintf(file, "- Potash-based fertilizer required: %.2f tons\n", potash_required);
    if (lime_required > 0) 
        fprintf(file, "- Lime required: %.2f tons\n", lime_required);
    if (sulfur_required > 0) 
        fprintf(file, "- Sulfur required: %.2f tons\n", sulfur_required);
    if (magnesium_sulfate_required > 0) 
        fprintf(file, "- Magnesium sulfate required: %.2f tons\n", magnesium_sulfate_required);
    if (calcium_sulfate_required > 0) 
        fprintf(file, "- Calcium sulfate required: %.2f tons\n", calcium_sulfate_required);

    fprintf(file, "Recovery Time: %d months\n", recovery_months);
    fprintf(file, "\nNote: Recovery times and specific measures depend on local conditions.\n");
    fprintf(file, "---------------------------------\n");
    fprintf(file, "End of Report\n");
}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef enum {
    MONDAY = 2, TUESDAY = 3, WEDNESDAY = 4, THURSDAY = 5, FRIDAY = 6, SATURDAY = 7
} Day;

typedef enum {
    MORNING = 1, AFTERNOON = 2
} AMPM;

typedef struct period_t {
    AMPM ampm;
    Day day;
    int from;
    int to;
    char weekStart[10];
    char weekEnd[10];
    char room[10];
} Period;

typedef struct course_t {
    char code[10];
    char name[30];
    Period period[10];
    int noPeriod;
} Course;

Course *getCourseFromFile(FILE *fp, int *numOfCourse);

Period parsePeriod(char *string);

void printCourse(Course course);

void printPeriod(Period period);

void printArrayOfCourse(Course *course, int numOfCourse);

int main(int argc, char *argv[]) {

    // read file
    char fileName[] = "input";
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: Cannot read file %s", fileName);
        return 1;
    }

//    Course *courseArray = getCourseFromFile(fp);
    int numOfCourse;
    Course *arr = getCourseFromFile(fp, &numOfCourse);

    printArrayOfCourse(arr, numOfCourse);

    return 0;
}

Period parsePeriod(char *string) {
    int indexToken = 1;
    const char delimiters[] = ",;";

    char *token;
    char *running = strdup(string);

    Period period;
    period.from = -1;
    period.to = -1;


    while ((token = strsep(&running, delimiters)) != NULL) {
        switch (indexToken) {
            // course code

            case 2: {
                int num = atoi(token);
                period.from = num % 10;
                period.ampm = (num / 10) % 10;
                period.day = (num / 100) % 10;
            }
                break;

            case 3: {
                int num = atoi(token);
                period.to = num % 10;
            }
                break;
            case 5:
                strcpy(period.weekStart, token);
                break;
            case 6:
                strcpy(period.weekEnd, token);
                break;

            case 7:
                strcpy(period.room, token);
                break;
            default:
                break;
        }

        indexToken++;


    }
    return period;
}

Course *getCourseFromFile(FILE *fp, int *numOfCourse) {
    char buffer[100];

    // count course
    int noCourse = 0;

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        noCourse++;
    }

    rewind(fp);

    Course *courseArr = (Course *) malloc(sizeof(Course) * noCourse);

    if (courseArr == NULL) {
        fprintf(stderr, "ERROR cannot malloc in %d:%s", __LINE__, __FILE__);
        exit(1);
    }

    *numOfCourse = noCourse;

    int i;
    for (i = 0; i < noCourse; i++) {
        courseArr[i].noPeriod = 0;
    }

    // parse
    int indexCourse = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        int indexToken = 0;

        buffer[strlen(buffer) - 1] = '\0';

        const char delimiters[] = "\t";

        char *token;
        char *running = strdup(buffer);
        while ((token = strsep(&running, delimiters)) != NULL) {

            switch (indexToken) {
                // course code
                case 0:
                    strcpy(courseArr[indexCourse].code, token);
                    break;
                case 1:
                    strcpy(courseArr[indexCourse].name, token);
                    break;
                default:
                    (courseArr[indexCourse].period[indexToken - 2]) = parsePeriod(token);

                    (courseArr[indexCourse].noPeriod)++;
                    break;

            }

            indexToken++;


        }

        indexCourse++;
    }

    return courseArr;
}

void printCourse(Course course) {

    printf("%-4s %-20s ", course.code, course.name);
    int i;
    for (i = 0; i < course.noPeriod; ++i) {
        printPeriod(course.period[i]);
    }
    printf("\n");
}

void printPeriod(Period period) {

    switch (period.ampm) {
        case MORNING:
            printf("%-10s", "Morning");
            break;
        case AFTERNOON:
            printf("%-10s", "Afternoon");
            break;
    }

    switch (period.day) {

        case MONDAY:
            printf("%-10s ", "Monday");
            break;
        case TUESDAY:
            printf("%-10s ", "Tuesday");
            break;
        case WEDNESDAY:
            printf("%-10s ", "Wednesday");
            break;
        case THURSDAY:
            printf("%-10s ", "Thursday");
            break;
        case FRIDAY:
            printf("%-10s ", "Friday");
            break;
        case SATURDAY:
            printf("%-10s ", "Saturday");
            break;
    }
    printf("%d %d %s %s %s ", period.from, period.to, period.weekStart, period.weekEnd, period.room);
}

void printArrayOfCourse(Course *course, int numOfCourse) {
    int i;
    for (i = 0; i < numOfCourse; ++i) {
        printCourse(course[i]);
    }
}
CS 300 Project Two : ABCU Advising Assistance Program
File : ProjectTwo.cpp

The program loads course data from a comma - separated input file, stores
courses in a vector, prints the courses in alphanumeric order, and looks
up an individual course with its prerequisite titles.

Expected input format :
COURSE - NUMBER, Course Title, Prerequisite - 1, Prerequisite - 2, ...

Example :
    CS - 101, Introduction to Programming
    CS - 201, Data Structures, CS - 101
    * /

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

    using namespace std;

struct Course {
    string courseNumber;
    string courseTitle;
    vector<string> prerequisites;
};

// Remove leading and trailing whitespace from a string.
string trim(const string& value) {
    const size_t first = value.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        return "";
    }

    const size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

// Convert a course number to uppercase for case-insensitive matching.
string normalizeCourseNumber(string courseNumber) {
    courseNumber = trim(courseNumber);
    transform(courseNumber.begin(), courseNumber.end(), courseNumber.begin(),
        [](unsigned char character) {
            return static_cast<char>(toupper(character));
        });
    return courseNumber;
}

// Split a CSV row on commas. The assignment input file uses comma-separated
// fields and does not require a separate CSV parsing library.
vector<string> splitCsvRow(const string& line) {
    vector<string> fields;
    string field;
    stringstream stream(line);

    while (getline(stream, field, ',')) {
        fields.push_back(trim(field));
    }

    // Preserve an empty final field if the line ends in a comma.
    if (!line.empty() && line.back() == ',') {
        fields.push_back("");
    }

    return fields;
}

// Read course data and populate the vector. Returns false if the file
// cannot be opened or if no valid course rows are found.
bool loadCourses(const string& fileName, vector<Course>& courses) {
    ifstream inputFile(fileName);
    if (!inputFile.is_open()) {
        cerr << "Error: Unable to open file '" << fileName << "'.\n";
        return false;
    }

    vector<Course> loadedCourses;
    string line;
    size_t lineNumber = 0;

    while (getline(inputFile, line)) {
        ++lineNumber;
        line = trim(line);

        if (line.empty()) {
            continue;
        }

        vector<string> fields = splitCsvRow(line);
        if (fields.size() < 2 || fields[0].empty() || fields[1].empty()) {
            cerr << "Warning: Skipping invalid row " << lineNumber
                << " (a course number and title are required).\n";
            continue;
        }

        Course course;
        course.courseNumber = normalizeCourseNumber(fields[0]);
        course.courseTitle = fields[1];

        for (size_t i = 2; i < fields.size(); ++i) {
            if (!fields[i].empty()) {
                course.prerequisites.push_back(normalizeCourseNumber(fields[i]));
            }
        }

        loadedCourses.push_back(course);
    }

    inputFile.close();

    if (loadedCourses.empty()) {
        cerr << "Error: No valid course records were found in '"
            << fileName << "'.\n";
        return false;
    }

    // Replace the current data only after a successful load.
    courses = loadedCourses;
    cout << "Successfully loaded " << courses.size() << " courses.\n";
    return true;
}

// Build an index for quick course lookups.
unordered_map<string, size_t> buildCourseIndex(const vector<Course>& courses) {
    unordered_map<string, size_t> courseIndex;

    for (size_t i = 0; i < courses.size(); ++i) {
        courseIndex[courses[i].courseNumber] = i;
    }

    return courseIndex;
}

// Print all courses sorted by course number from lowest to highest.
void printCourseList(const vector<Course>& courses) {
    if (courses.empty()) {
        cout << "No course data is loaded. Please select option 1 first.\n";
        return;
    }

    vector<Course> sortedCourses = courses;
    sort(sortedCourses.begin(), sortedCourses.end(),
        [](const Course& left, const Course& right) {
            return left.courseNumber < right.courseNumber;
        });

    cout << "\nHere is a list of all courses in the Computer Science program:\n";
    for (const Course& course : sortedCourses) {
        cout << course.courseNumber << ", " << course.courseTitle << '\n';
    }
    cout << '\n';
}

// Print a course's title and prerequisites. Prerequisite titles are found
// by matching each prerequisite number against the loaded course index.
void printCourseInformation(
    const vector<Course>& courses,
    const unordered_map<string, size_t>& courseIndex) {

    if (courses.empty()) {
        cout << "No course data is loaded. Please select option 1 first.\n";
        return;
    }

    string requestedNumber;
    cout << "What course do you want to know about? ";
    getline(cin, requestedNumber);
    requestedNumber = normalizeCourseNumber(requestedNumber);

    auto coursePosition = courseIndex.find(requestedNumber);
    if (coursePosition == courseIndex.end()) {
        cout << "Error: Course '" << requestedNumber
            << "' was not found. Please check the course number and try again.\n\n";
        return;
    }

    const Course& course = courses[coursePosition->second];
    cout << course.courseNumber << ", " << course.courseTitle << '\n';

    if (course.prerequisites.empty()) {
        cout << "Prerequisites: None\n\n";
        return;
    }

    cout << "Prerequisites:\n";
    for (const string& prerequisiteNumber : course.prerequisites) {
        auto prerequisitePosition = courseIndex.find(prerequisiteNumber);

        if (prerequisitePosition != courseIndex.end()) {
            const Course& prerequisite = courses[prerequisitePosition->second];
            cout << "  " << prerequisite.courseNumber << ", "
                << prerequisite.courseTitle << '\n';
        }
        else {
            // Show the prerequisite number even if its course row is missing.
            cout << "  " << prerequisiteNumber
                << ", title unavailable (course not found in loaded file)\n";
        }
    }
    cout << '\n';
}

void displayMenu() {
    cout << "Welcome to the ABCU Advising Assistance Program\n\n";
    cout << "  1. Load Course Data\n";
    cout << "  2. Print Course List\n";
    cout << "  3. Print Course Information\n";
    cout << "  9. Exit\n\n";
    cout << "What would you like to do? ";
}

int main() {
    vector<Course> courses;
    unordered_map<string, size_t> courseIndex;
    bool dataLoaded = false;
    string choiceText;

    while (true) {
        displayMenu();
        getline(cin, choiceText);
        choiceText = trim(choiceText);

        if (choiceText == "1") {
            string fileName;
            cout << "Enter the course data file name: ";
            getline(cin, fileName);
            fileName = trim(fileName);

            if (fileName.empty()) {
                cout << "Error: The file name cannot be empty.\n\n";
                continue;
            }

            if (loadCourses(fileName, courses)) {
                courseIndex = buildCourseIndex(courses);
                dataLoaded = true;
            }
            cout << '\n';
        }
        else if (choiceText == "2") {
            if (!dataLoaded) {
                cout << "No course data is loaded. Please select option 1 first.\n\n";
            }
            else {
                printCourseList(courses);
            }
        }
        else if (choiceText == "3") {
            if (!dataLoaded) {
                cout << "No course data is loaded. Please select option 1 first.\n\n";
            }
            else {
                printCourseInformation(courses, courseIndex);
            }
        }
        else if (choiceText == "9") {
            cout << "Thank you for using the ABCU Advising Assistance Program!\n";
            break;
        }
        else {
            cout << "Invalid option. Please enter 1, 2, 3, or 9.\n\n";
        }
    }

    return 0;
}

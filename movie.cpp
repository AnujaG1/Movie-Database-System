#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
using namespace std;

class Movie {
public:
    string title;
    int year;
    string director;
    string genre;
    vector<string> cast;
    string plotSummary;
    map<string, vector<int>> userRatings; // username -> ratings
    map<string, string> userReviews; // username -> review

    Movie(string t, int y, string d, string g, vector<string> c, string p)
        : title(t), year(y), director(d), genre(g), cast(c), plotSummary(p) {}

    string toString() const {
        stringstream ss;
        ss << "{";
        ss << "\"title\": \"" << title << "\", ";
        ss << "\"year\": " << year << ", ";
        ss << "\"director\": \"" << director << "\", ";
        ss << "\"genre\": \"" << genre << "\", ";
        ss << "\"cast\": [";
        for (size_t i = 0; i < cast.size(); ++i) {
            ss << "\"" << cast[i] << "\"";
            if (i < cast.size() - 1) ss << ", ";
        }
        ss << "], ";
        ss << "\"plotSummary\": \"" << plotSummary << "\"";
        ss << "}";
        return ss.str();
    }
    static Movie fromString(const string &str) {
        stringstream ss(str);
        string title, director, genre, plotSummary;
        int year;
        vector<string> cast;
        ss.ignore(9); // Skip '{"title": "'
        getline(ss, title, '"');
        ss.ignore(10); // Skip '", "year": '
        ss >> year;
        ss.ignore(12); // Skip ', "director": "'
        getline(ss, director, '"');
        ss.ignore(10); // Skip ', "genre": "'
        getline(ss, genre, '"');
        ss.ignore(9); // Skip ', "cast": ['
        string actor;
        while (getline(ss, actor, ',')) {
            actor = actor.substr(1, actor.size() - 2); // Remove quotes
            cast.push_back(actor);
            if (ss.peek() == ']') break;
            ss.ignore(2); // Skip ', '
        }
        ss.ignore(16); // Skip '], "plotSummary": "'
        getline(ss, plotSummary, '"');
        return Movie(title, year, director, genre, cast, plotSummary);
    }
};

class MovieDatabase {
private:
    map<string, vector<Movie>> userLibraries; // username -> list of movies
    const string filename = "movies.json";

    void loadFromFile() {
        ifstream inFile(filename);
        string line;
        while (getline(inFile, line)) {
            if (!line.empty()) {
                Movie movie = Movie::fromString(line);
                userLibraries["default"].push_back(movie); // Load movies to default user library
            }
        }
        inFile.close();
    }

    void saveToFile() {
        ofstream outFile(filename);
        if (outFile.is_open()) {
            for (const auto &user : userLibraries) {
                for (const auto &movie : user.second) {
                    outFile << movie.toString() << endl;
                }
            }
            outFile.close();
        } else {
            cout << "Unable to open file for writing." << endl;
        }
    }

public:
    MovieDatabase() {
        loadFromFile();
    }

    ~MovieDatabase() {
        saveToFile();
    }

    void addMovie(const string &username, const Movie &movie) {
        userLibraries[username].push_back(movie);
    }

    void editMovie(const string &username, const string &title, const Movie &updatedMovie) {
        for (auto &movie : userLibraries[username]) {
            if (movie.title == title) {
                movie = updatedMovie;
                return;
            }
        }
    }

    void deleteMovie(const string &username, const string &title) {
        auto &movies = userLibraries[username];
        movies.erase(remove_if(movies.begin(), movies.end(), [&](const Movie &movie) {
            return movie.title == title;
        }), movies.end());
    }

    void searchMovie(const string &username, const string &title) {
        for (const auto &movie : userLibraries[username]) {
            if (movie.title == title) {
                cout << "Found: " << movie.toString() << endl;
                return;
            }
        }
        cout << "Movie not found!" << endl;
    }

    void sortMovies(const string &username, const string &criteria) {
        auto &movies = userLibraries[username];
        if (criteria == "title") {
            sort(movies.begin(), movies.end(), [](const Movie &a, const Movie &b) {
                return a.title < b.title;
            });
        } else if (criteria == "year") {
            sort(movies.begin(), movies.end(), [](const Movie &a, const Movie &b) {
                return a.year < b.year;
            });
        } else if (criteria == "director") {
            sort(movies.begin(), movies.end(), [](const Movie &a, const Movie &b) {
                return a.director < b.director;
            });
        } else if (criteria == "genre") {
            sort(movies.begin(), movies.end(), [](const Movie &a, const Movie &b) {
                return a.genre < b.genre;
            });
        }
    }

    void filterMovies(const string &username, const string &criteria, const string &value) {
        for (const auto &movie : userLibraries[username]) {
            if ((criteria == "title" && movie.title == value) ||
                (criteria == "year" && to_string(movie.year) == value) ||
                (criteria == "director" && movie.director == value) ||
                (criteria == "genre" && movie.genre == value)) {
                cout << movie.toString() << endl;
            }
        }
    }

    void rateMovie(const string &username, const string &title, int rating) {
        for (auto &movie : userLibraries[username]) {
            if (movie.title == title) {
                movie.userRatings[username].push_back(rating);
                return;
            }
        }
        cout << "Movie not found!" << endl;
    }

    void reviewMovie(const string &username, const string &title, const string &review) {
        for (auto &movie : userLibraries[username]) {
            if (movie.title == title) {
                movie.userReviews[username] = review;
                return;
            }
        }
        cout << "Movie not found!" << endl;
    }

    void recommendMovies(const string &username) {
        // Simple recommendation based on most frequently rated movies
        map<string, int> movieFrequency;
        for (const auto &user : userLibraries) {
            for (const auto &movie : user.second) {
                auto it = movie.userRatings.find(username);
             if (it != movie.userRatings.end() && !it->second.empty()) {
                    movieFrequency[movie.title]++;
                }
            }
        }
        vector<pair<string, int>> recommendations(movieFrequency.begin(), movieFrequency.end());
        sort(recommendations.begin(), recommendations.end(), [](const pair<string, int> &a, const pair<string, int> &b) {
            return b.second < a.second;
        });
        cout << "Recommendations for " << username << ":" << endl;
        for (const auto &rec : recommendations) {
            cout << rec.first << " (" << rec.second << " ratings)" << endl;
        }
    }

    void importMovies(const string &filename, const string &username) {
        ifstream inFile(filename);
        string line;
        while (getline(inFile, line)) {
            if (!line.empty()) {
                userLibraries[username].push_back(Movie::fromString(line));
            }
        }
        inFile.close();
    }

    void exportMovies(const string &filename, const string &username) {
        ofstream outFile(filename);
        if (outFile.is_open()) {
            for (const auto &movie : userLibraries[username]) {
                outFile << movie.toString() << endl;
            }
            outFile.close();
        } else {
            cout << "Unable to open file for writing." << endl;
        }
    }
};
int main() {
    MovieDatabase db;
    int choice;
    string username, title, director, genre, criteria, value, review, castMember;
    int year, rating;
    vector<string> cast;
    string plotSummary;
       
    while (true) {
        cout << "1. Add Movie\n2. Edit Movie\n3. Delete Movie\n4. Search Movie";
        cout<<"\n5. Sort Movies\n6. Filter Movies\n7. Rate Movie\n8. Review Movie\n9. Recommend Movies";
        cout<<"\n10. Import Movies\n11. Export Movies\n12. Exit\nChoose an option: ";
        cin >> choice;
        cin.ignore();

        switch (choice) {
        case 1:
            cout << "Enter username: "; getline(cin, username);
            cout << "Enter title: "; getline(cin, title);
            cout << "Enter year: "; cin >> year; cin.ignore();
            cout << "Enter director: "; getline(cin, director);
            cout << "Enter genre: "; getline(cin, genre);
            cout << "Enter plot summary: "; getline(cin, plotSummary);
            cout << "Enter cast members (comma separated): ";
            getline(cin, castMember);
            {
                stringstream ss(castMember);
                string actor;
                while (getline(ss, actor, ',')) {
                    cast.push_back(actor);
                }
            }
            db.addMovie(username, Movie(title, year, director, genre, cast, plotSummary));
            break;
        case 2:
            cout << "Enter username: "; getline(cin, username);
            cout << "Enter title of movie to edit: "; getline(cin, title);
            cout << "Enter new title: "; getline(cin, title);
            cout << "Enter new year: "; cin >> year; cin.ignore();
            cout << "Enter new director: "; getline(cin, director);
            cout << "Enter new genre: "; getline(cin, genre);
            cout << "Enter new plot summary: "; getline(cin, plotSummary);
            cout << "Enter new cast members (comma separated): ";
            getline(cin, castMember);
            cast.clear();
            {

                stringstream ss(castMember);
                string actor;
                while (getline(ss, actor, ',')) {
                    cast.push_back(actor);
                }
            }
            db.editMovie(username, title, Movie(title, year, director, genre, cast, plotSummary));
            break;
        case 3:
            cout << "Enter username: "; getline(cin, username);
            cout << "Enter title of movie to delete: "; getline(cin, title);
            db.deleteMovie(username, title);
            break;
        case 4:
            cout << "Enter username: "; getline(cin, username);
            cout << "Enter title of movie to search: "; getline(cin, title);
            db.searchMovie(username, title);
            break;
        case 5:
            cout << "Enter username: "; getline(cin, username);
            cout << "Sort by (title/year/director/genre): "; getline(cin, criteria);
            db.sortMovies(username, criteria);
            break;
        case 6:
            cout << "Enter username: "; getline(cin, username);
            cout << "Filter by (title/year/director/genre): "; getline(cin, criteria);
            cout << "Enter value: "; getline(cin, value);
            db.filterMovies(username, criteria, value);
            break;
        case 7:
            cout << "Enter username: "; getline(cin, username);
            cout << "Enter title of movie to rate: "; getline(cin, title);
            cout << "Enter your rating (1-5): "; cin >> rating; cin.ignore();
            db.rateMovie(username, title, rating);
            break;
        case 8:
            cout << "Enter username: "; getline(cin, username);
            cout << "Enter title of movie to review: "; getline(cin, title);
            cout << "Enter your review: "; getline(cin, review);
            db.reviewMovie(username, title, review);
            break;
        case 9:
            cout << "Enter username: "; getline(cin, username);
            db.recommendMovies(username);
            break;
        case 10:
            cout << "Enter username: "; getline(cin, username);
            cout << "Enter filename to import from: "; getline(cin, value);
            db.importMovies(value, username);
            break;
        case 11:
            cout << "Enter username: "; getline(cin, username);
            cout << "Enter filename to export to: "; getline(cin, value);
            db.exportMovies(value, username);
            break;
        case 12:
            return 0;
        default:
            cout << "Invalid option! Try again." << endl;
        }
    }

    return 0;
}
#include <iostream>
#include <vector>
#include <string>
using namespace std;

enum keyMap {
    ORIGNAL = 1,
    ADD = 0
};

struct Piece {
    int index;
    int length;
    bool location;
    Piece(int s, int l, bool buff) : index(s), length(l), location(buff) {}
};

class PieceTable {
private:
    string original;
    string buffer;
    vector<Piece> pieces;

public:
    PieceTable(const string& text) {
        original = text;
        Piece origin = Piece(0, text.length(), ORIGNAL);
        pieces.push_back(origin);
    }

    void insert(int index, const string& s) {
        Piece newPiece = Piece(buffer.length(), s.length(), ADD);
        buffer += s;
        vector<Piece> newPieces;
        int pieceLength = 0;
        for (int x = 0;x < pieces.size();x++) {
            cout << "here";
            pieceLength += pieces[x].length;
            cout << pieceLength - pieces[x].length << endl;
            if (pieceLength == index) {
                newPieces.push_back(newPiece);
            }
           
            else if (pieceLength > index && pieceLength - pieces[x].length < index) {
                cout<<"here";
                newPieces.push_back(Piece(pieces[x].index, pieces[x]. - s.length(), pieces[x].location));
                newPieces.push_back(newPiece);
                newPieces.push_back(Piece(pieces[x].index + s.length(), pieces[x].length - (pieces[x].index - s.length()), pieces[x].location));
            }
            else {
                newPieces.push_back(pieces[x]);
            }
        }
        pieces = newPieces;





        // int x;





        // for (x = 0;x < pieces.size();x++) {
        //     if (pieces[x].index >= index) {
        //         pieces[x].index += s.length();
        //     }
        //     else if (pieces[x].index + pieces[x].length >= s.length()) {
        //         int lengthCopy = pieces[x].length;
        //         pieces[x].length = index - pieces[x].index;
        //         pieces.push_back(Piece(index + s.length(), lengthCopy - pieces[x].length, pieces[x].location));
        //     }
        // }
    }


    string getText() const {
        cout << original << endl << buffer << endl;
        for (auto it : pieces) {
            cout << it.index << " " << it.length << " " << it.location << endl;
        }
        return "";
    }
};

int main() {
    PieceTable pt("Hello, World!");

    // Inserting into the middle of the document
    pt.insert(7, "beautiful ");

    // Inserting at the end of the document


    std::cout << "Resulting text: " << pt.getText() << std::endl;

    return 0;
}

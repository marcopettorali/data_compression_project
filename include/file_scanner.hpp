#include <fstream>
#include <iostream>
#include <vector>

class TextFileIterator {

    std::ifstream m_txt_stream;
    uint8_t cur_char;
    bool m_eof;
    const char* m_filename;

public:
    const char* filename() const{
        return m_filename;
    }

    TextFileIterator(const char* filename) : m_filename(filename) {
        m_txt_stream = std::ifstream(filename, std::ifstream::in | std::ifstream::binary);
        m_txt_stream >> std::noskipws;

        if(!m_txt_stream.is_open()) {
            std::cerr<<"Error! Could not open file "<<filename<<std::endl;
            exit(-1);
        }

        if(m_txt_stream.peek() == std::ifstream::traits_type::eof()) {
            std::cerr<<"The submitted file "<<filename<<" is empty. Exiting"<<std::endl;
            exit(-1);
        }

        m_eof = true;
        if (m_txt_stream >> cur_char) {
            m_eof = false;
        }
    }

    bool end() {
        return m_eof;
    }

    uint64_t operator*() {
        return (uint64_t)cur_char;
    }

    void operator++() {
        if(m_eof) {
            std::cerr<<"TextFileIterator Error: Calling next at the end of the stream"<<std::endl;
            exit(-1);  
        }
        if (m_txt_stream >> cur_char) {
            m_eof = false;
        }
        else {
            m_eof = true;
        }
    }

    TextFileIterator (TextFileIterator const& other) : TextFileIterator(other.filename()) {}

    TextFileIterator operator= (const TextFileIterator &other) {
        return TextFileIterator(other.filename());
    }

    // get remaining characters as a std vector and close this iterator
    std::vector<uint8_t> as_vector() {
        std::vector<uint8_t> m_vec;
        while(!this->end()) {
            m_vec.push_back(**this);
            ++(*this);
        }
        this->close();
        return m_vec;
    }

    void close() {
        m_txt_stream.close();
    }
};
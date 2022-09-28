#include<iostream>
#include "dacs/constants.h"
#include "dacs/dacs.c"

#ifdef NAT_LANG_OPT
#include "nat_lang_optimizer.hpp"
#endif


namespace tongrams {

    struct directly_accessable_code_sequence {
        directly_accessable_code_sequence() : listaux(NULL) {}

        template <typename Iterator>
        // if n is higher than the list size, then the iterator is moved forward until its end. Requires a boolean end() method on the iterator.
        void build(Iterator begin, uint64_t n, uint8_t L_max) {
            levels_max = L_max;
            listaux = NULL;
            auto m_vec = begin.as_vector();
            auto end = m_vec.size() < n ? m_vec.size() : n;
            uint* m_vec_chars = new uint[end];
            for(uint i = 0; i<end; ++i) {
                #ifdef NAT_LANG_OPT
                    m_vec_chars[i] = (uint)vtocw_table[m_vec[i]];
                #else
                    m_vec_chars[i] = (uint)m_vec[i];
                #endif
            }

            listRep = createFT(m_vec_chars, end, L_max);
            delete[] m_vec_chars;
            
        }

        inline uint64_t operator[](uint64_t i) const {
            uint value;
            value = accessFT(listRep, i+1);
            #ifdef NAT_LANG_OPT
                return cwtov_table[(uint8_t)value];
            #else
                return (uint8_t) value;
            #endif
        }

        inline uint64_t size() {
            return (uint64_t)listRep->listLength;
        }

        inline uint64_t bytes() {
            auto rep = listRep;
            auto br=listRep->bS;
            auto s=br->s;
	        auto n=br->n;
            uint64_t bytes = sizeof(uint)*(rep->nLevels*4 + rep->tamtablebase + 4 + (rep->tamCode/W+1)) + sizeof(ushort)*rep->nLevels;
            bytes += (2 + n/W+1 + n/s+1)*sizeof(uint);
            return bytes;
        }

        struct iterator {
            iterator(directly_accessable_code_sequence const* seq, uint64_t pos)
                : m_seq(seq), m_pos(pos), m_len(0) {
                aux = decompressFT(seq->listRep, seq->listRep->listLength);
            }

            uint64_t operator*() {
                return cwtov_table[(uint8_t) aux[m_pos]];
            }

            void operator++() {
                ++m_pos;
            }

            bool operator==(iterator const& other) const {
                return m_pos == other.m_pos;
            }

            bool operator!=(iterator const& other) const {
                return !(*this == other);
            }

        private:
            directly_accessable_code_sequence const* m_seq;
            uint64_t m_pos;
            uint64_t m_len;
            uint* aux;
        };

        iterator begin() {
            return iterator(this, 0);
        }

        iterator end() const {
            return iterator(this, listRep->listLength);
        }

        void save(char* filename) const {
            saveFT(listRep, filename);
        }

        void load(char* filename) {
            listRep = loadFT(filename);
        }

    private: 
        FTRep* listRep;
        uint* listaux;
        uint8_t levels_max;
    };
};
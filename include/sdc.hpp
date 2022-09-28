#pragma once

#include "util.hpp"
#include "bit_vector.hpp"
#include "select_support_array.hpp"

#ifdef NAT_LANG_OPT
#include "nat_lang_optimizer.hpp"
#endif

namespace tongrams {

    struct simple_dense_code_sequence {
        simple_dense_code_sequence() : m_size(0), chunk_size(4) {}

        template <typename Iterator>
        // if n is higher than the list size, then the iterator is moved forward until its end. Requires a boolean end() method on the iterator.
        void build(Iterator begin, uint64_t n, uint8_t bits_per_chunk=4) {

            chunk_size = bits_per_chunk;
            m_size = n;
            auto start = begin;
            uint64_t bits = 0;
            uint64_t i;
            for (i = 0; i < n && !start.end(); ++i, ++start) {
                uint64_t min_bits = floor_log2(encode_cw(*start)) + 1;
                uint64_t len = chunk_size* (uint64_t) ceil(double(min_bits)/chunk_size);
                assert(len <= 64);
                bits += len;
            }
            if(start.end()) {
                m_size = n = i;
            }
            
            bit_vector_builder bvb_index((bits/chunk_size) + 1);
            bit_vector_builder bvb_codewords(bits);

            uint64_t pos = 0;
            for (uint64_t i = 0; i < n; ++i, ++begin) {
                auto v = *begin;
                uint64_t cw = encode_cw(v);
                uint64_t min_bits = floor_log2(cw) + 1;
                uint64_t len = chunk_size* (uint64_t) ceil(double(min_bits)/chunk_size);
                assert(len <= 64);
                bvb_codewords.set_bits(pos, cw, len);
                bvb_index.set(pos/chunk_size, 1);
                pos += len;
            }
            // NOTE: store a last 1 to delimit last codeword:
            // avoid test for last codeword in operator[]
            assert(pos == bits);
            bvb_index.set(pos/chunk_size, 1);

            bit_vector bv_cw, bv_idx;
            bvb_codewords.build(bv_cw);
            bvb_index.build(bv_idx);

            bv_cw.swap(m_codewords);
            bv_idx.swap(m_index);
            select_support_array(m_index).swap(m_index_d1);

        }

        inline uint64_t operator[](uint64_t i) const {
            uint64_t pos = m_index_d1.select(m_index, i);
            assert(pos < m_index.size());
            bit_vector::unary_iterator e(m_index, pos + 1);
            uint64_t next = e.next();
            uint64_t len = (next - pos)*chunk_size;
            assert(len <= 64);
            uint64_t cw = m_codewords.get_bits(pos*chunk_size, len);
            uint64_t value = decode_cw(cw);
            return value;
        }

        struct iterator {
            iterator(simple_dense_code_sequence const* seq, uint64_t pos)
                : m_seq(seq), m_pos(pos), m_len(0) {
                m_it = bit_vector::unary_iterator(m_seq->m_index, pos);

                // remember that you have to put a leading 1 in m_index for this to work!
                m_pos = m_it.next();
            }

            uint64_t operator*() {
                m_len = (m_it.next() - m_pos)*(m_seq->chunk_size);
                assert(m_len <= 64);
                uint64_t cw = m_seq->m_codewords.get_bits(m_pos*(m_seq->chunk_size), m_len);
                // the extracted codeword is the value itself, we can return it!
                return m_seq->decode_cw(cw);
            }

            void operator++() {
                m_pos += (m_len/(m_seq->chunk_size));
            }

            bool operator==(iterator const& other) const {
                return m_pos == other.m_pos;
            }

            bool operator!=(iterator const& other) const {
                return !(*this == other);
            }

    private:
        simple_dense_code_sequence const* m_seq;
        uint64_t m_pos;
        uint64_t m_len;
        bit_vector::unary_iterator m_it;
    };

    iterator begin() const {
        return iterator(this, 0);
    }

    iterator end() const {
        return iterator(this, m_codewords.size());
    }

    uint64_t size() const {
        return m_size;
    }

    uint8_t chunk() const {
        return chunk_size;
    }

    void save(std::ostream& os) const {
        essentials::save_pod(os, m_size);
        m_codewords.save(os);
        m_index.save(os);
        m_index_d1.save(os);
    }

    void load(std::istream& is) {
        essentials::load_pod(is, m_size);
        m_codewords.load(is);
        m_index.load(is);
        m_index_d1.load(is);
    }

    uint64_t bytes() const {
        return sizeof(m_size) + m_codewords.bytes() + m_index.bytes() +
               m_index_d1.bytes();
    }

private:
    uint64_t m_size;
    bit_vector m_codewords;
    bit_vector m_index;
    select_support_array m_index_d1;
    uint8_t chunk_size;

    inline uint64_t encode_cw(uint64_t const& value) const {
        #ifdef NAT_LANG_OPT
            //return (value - uint64_t('a')) % uint64_t(256);
            return uint64_t(vtocw_table[value]);
        #else
            return value;
        #endif
    }

    inline uint64_t decode_cw(uint64_t const& cw) const {
        #ifdef NAT_LANG_OPT
            //return (cw + uint64_t('a')) % uint64_t(256);
            return uint64_t(cwtov_table[cw]);
        #else
            return cw;
        #endif
    }

    };

};
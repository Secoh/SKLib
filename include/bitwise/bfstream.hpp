// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Application of bit stream abstratction - file based serialized bit packs I/O
// This is internal SKLib file and must NOT be included directly.

// -------------------------------------------------------------------

//TODO: must be "open" member function

class bits_file_type : public sklib::bits_stream_base_type
{
private:
    std::fstream fs;
    std::ios_base::openmode fs_mode;

    bool is_readable() const  { return (fs_mode & std::ios_base::in); }
    bool is_writeable() const { return (fs_mode & std::ios_base::out); }

public:
    template<class T, std::enable_if_t<sklib::is_any_string<T>, bool> = true>
    explicit bits_file_type(const T& filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
        : sklib::bits_stream_base_type(read_byte_proc, write_byte_proc, stream_action)
        , fs_mode(mode)
        , fs(filename, mode | std::ios_base::binary)
    {
        if (is_readable() && is_writeable() && !fs.is_open())  // extend RW mode: if file doesn't exist, create
        {
            fs.open(filename, std::ios_base::out);
            fs.close();
            fs.open(filename, mode | std::ios_base::binary);
        }
    }

    std::fstream& file_stream() { return fs; }

private:
    bool redirect_read_byte(uint8_t& data)
    {
        if (!is_readable() || fs.eof()) return false;

        char c = 0;
        fs.read(&c, 1);
        if (fs.eof()) return false;

        data = c;
        return true;
    }
    static bool read_byte_proc(sklib::bits_stream_base_type* root, uint8_t& data)
    {
        return static_cast<bits_file_type*>(root)->redirect_read_byte(data);
    }

    void redirect_write_byte(uint8_t data)
    {
        char c = data;
        if (is_writeable()) fs.write(&c, 1);
    }
    static void write_byte_proc(sklib::bits_stream_base_type* root, uint8_t data)
    {
        static_cast<bits_file_type*>(root)->redirect_write_byte(data);
    }

    void action_after_reset()
    {
        if (is_readable()) fs.seekg(0);
        if (is_writeable()) fs.seekp(0);
    }
    void action_after_flush()
    {
        if (is_writeable()) fs.flush();
    }
    void action_before_rewind()
    {
        if (is_readable()) fs.seekg(0);
    }
    static void stream_action(sklib::bits_stream_base_type* root, hook_type what)
    {
        auto self = static_cast<bits_file_type*>(root);

        switch (what)
        {
        case hook_type::after_reset:   self->action_after_reset();   break;
        case hook_type::after_flush:   self->action_after_flush();   break;
        case hook_type::before_rewind: self->action_before_rewind(); //break;
        }
    }
};


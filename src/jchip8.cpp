/***************************************************************************
 *   Copyright (C) 2005 by Jeff Ferr                                       *
 *   root@sat                                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330x00, Boston, MA  02111-1307, USA.          *
 ***************************************************************************/
#include "jgui/japplication.h"
#include "jgui/jwindow.h"
#include "jgui/jbufferedimage.h"

#undef JDEBUG_ENABLED
#include "jlogger/jloggerlib.h"

#include <iostream>

unsigned char chip8_fontset[80] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

class Chip8 : public jgui::Window {

	private:
    static constexpr jgui::jsize_t<int>
      _screen {64, 32};

    uint8_t
      _memory[4096] {},
      _input[16] {},
      _video[_screen.width*_screen.height] {},
      _key[16] {},
      _register[16] {};
    uint16_t
      _stack[16] {};
    uint16_t
      _index = 0,
      _program_counter = 0,
      _stack_pointer = 0,
      _delay_timer = 0,
      _sound_timer = 0;

	public:
		Chip8():
			jgui::Window({720, 480})
		{
      for (int i=0; i<4096; i++) {
        _memory[i] = 0;
      }
      
      for(int i=0; i<80; i++) {
        _memory[i] = chip8_fontset[i];
      }

      for (int i=0; i<_screen.width*_screen.height; i++) {
        _video[i] = 0;
      }

      for (int i=0; i<16; i++) {
        _stack[i] = 0;
        _key[i] = 0;
      }

      _delay_timer = 0;
      _sound_timer = 0;
		}

		virtual ~Chip8()
		{
		}

    void Process()
    {
      uint16_t
        opcode = (_memory[_program_counter++] << 0x08) | (_memory[_program_counter++] << 0x00),
        nnn = opcode & 0x0fff;
      uint8_t
        x = (opcode >> 8) & 0x0f,
        y = (opcode >> 4) & 0x0f,
        n = (opcode >> 0) & 0x0f,
        nn = opcode & 0xff;
      uint8_t
        *vx = &_register[x],
        *vy = &_register[y];

      switch (opcode & 0xf000) {
        case 0x0000: 
          switch (opcode & 0x000f) {
            case 0x0000: // 00E0
              JDEBUG(JINFO, "00E0: %04x\n", opcode);

              for (int j=0; j<_screen.height; j++) {
                for (int i=0; i<_screen.width; i++) {
                  _video[j*_screen.width + i] = 0;
                }
              }

              break;
            case 0x000e: // 00EE
              JDEBUG(JINFO, "00EE: %04x\n", opcode);
              
              _program_counter = _stack[--_stack_pointer];

              break;
          }
          break;
        case 0x1000: // 1NNN
          JDEBUG(JINFO, "1NNN: %04x\n", opcode);

          _program_counter = nnn;

          break;
        case 0x2000: // 2NNN
          JDEBUG(JINFO, "2NNN: %04x\n", opcode);

          _stack[_stack_pointer++] = _program_counter;
          _program_counter = nnn;

          break;
        case 0x3000: // 3XNN
          JDEBUG(JINFO, "3XNN: %04x\n", opcode);

          if (*vx == nn) {
            _program_counter += 2;
          }

          break;
        case 0x4000: // 4XNN
          JDEBUG(JINFO, "4XNN: %04x\n", opcode);

          if (*vx != nn) {
            _program_counter += 2;
          }

          break;
        case 0x5000: // 5XY0
          JDEBUG(JINFO, "5XY0: %04x\n", opcode);

          if (*vx == *vy) {
            _program_counter += 2;
          }

          break;
        case 0x6000: // 6XNN
          JDEBUG(JINFO, "6XNN: %04x\n", opcode);

          *vx = nn;

          break;
        case 0x7000: // 7XNN
          JDEBUG(JINFO, "7XNN: %04x\n", opcode);

          *vx += nn;

          break;
        case 0x8000: 
          switch (opcode & 0x000f) {
            case 0x0000: // 8XY0
              JDEBUG(JINFO, "8XY0: %04x\n", opcode);

              *vx = *vy;

              break;
            case 0x0001: // 8XY1
              JDEBUG(JINFO, "8XY1: %04x\n", opcode);

              *vx |= *vy;

              break;
            case 0x0002: // 8XY2
              JDEBUG(JINFO, "8XY2: %04x\n", opcode);

              *vx &= *vy;

              break;
            case 0x0003: // 8XY3
              JDEBUG(JINFO, "8XY3: %04x\n", opcode);

              *vx ^= *vy;

              break;
            case 0x0004: // 8XY4
              JDEBUG(JINFO, "8XY4: %04x\n", opcode);

              _register[0x0f] = 0;

              if (*vy > (0xff - *vx)) {
                _register[0x0f] = 1;
              }

              *vx += *vy;

              break;
            case 0x0005: // 8XY5
              JDEBUG(JINFO, "8XY5: %04x\n", opcode);

              _register[0x0f] = 0;

              if (*vx > *vy) {
                _register[0x0f] = 1;
              }

              *vx -= *vy;

              break;
            case 0x0006: // 8XY6
              JDEBUG(JINFO, "8XY6: %04x\n", opcode);

              _register[0x0f] = *vx & 0x01;
              *vx >>= 1;

              break;
            case 0x0007: // 8XY7
              JDEBUG(JINFO, "8XY7: %04x\n", opcode);

              _register[0x0f] = 0;

              if (*vy > *vx) {
                _register[0x0f] = 1;
              }

              *vx = *vy - *vx;

              break;
            case 0x000e: // 8XYE
              JDEBUG(JINFO, "8XYE: %04x\n", opcode);

              _register[0x0f] = *vx >> 7;
              *vx <<= 1;

              break;
          }
          break;
        case 0x9000: // 9XY0
          JDEBUG(JINFO, "9XY0: %04x\n", opcode);

          if (*vx != *vy) {
            _program_counter += 2;
          }

          break;
        case 0xa000: // ANNN
          JDEBUG(JINFO, "ANNN: %04x\n", opcode);

          _index = opcode & 0x0fff;

          break;
        case 0xb000: // BNNN
          JDEBUG(JINFO, "BNNN: %04x\n", opcode);

          _program_counter = _register[0x00] + nnn;

          break;
        case 0xc000: // CXNN
          JDEBUG(JINFO, "CXNN: %04x\n", opcode);

          *vx = (random() % 0xff) & nn;
          
          break;
        case 0xd000: { // DXYN
            JDEBUG(JINFO, "DXYN: %04x\n", opcode);

            uint16_t
              x0 = *vx,
              y0 = *vy;

            _register[0x0f] = 0;

            for (int yline=0; yline<n; yline++) {
              for (int xline=0; xline<8; xline++) {
                if ((_memory[_index + yline] & (0x80 >> xline)) != 0) {
                  if (_video[(y0 + yline)*_screen.width + x0 + xline] == 1) {
                    _register[0x0f] = 1;
                  }

                  _video[(y0 + yline)*_screen.width + x0 + xline] ^= 1;
                }
              }
            }

            Repaint();

            jgui::Application::FrameRate(30);
          }

          break;
        case 0xe000: // key instructions
          switch (opcode & 0x00ff) {
            case 0x009e: // EX9E
              JDEBUG(JINFO, "EX9E: %04x\n", opcode);

              if (_key[*vx] != 0) {
                _program_counter += 2;
              }

              break;
            case 0x00a1: // EXA1
              JDEBUG(JINFO, "EXA1: %04x\n", opcode);

              if (_key[*vx] == 0) {
                _program_counter += 2;
              }

              break;
          }

          break;
        case 0xf000: 
          switch (opcode & 0x00ff) {
            case 0x0007: // FX07
              JDEBUG(JINFO, "FX07: %04x\n", opcode);

              *vx = _delay_timer;

              break;
            case 0x000a: // FX0A // key instruction
              JDEBUG(JINFO, "FX0A: %04x\n", opcode);

              {
                bool keyPress = false;

                for (int i=0; i<16; i++) {
                  if (_key[i] != 0) {
                    *vx = i;

                    keyPress = true;
                  }
                }
                
                if (!keyPress) {
                  _program_counter -= 2;

                  return;
                }
              }

              break;
            case 0x0015: // FX15
              JDEBUG(JINFO, "FX15: %04x\n", opcode);

              _delay_timer = *vx;

              break;
            case 0x0018: // FX18
              JDEBUG(JINFO, "FX18: %04x\n", opcode);

              _sound_timer = *vx;

              break;
            case 0x001e: // FX1E
              JDEBUG(JINFO, "FX1E: %04x\n", opcode);

              _register[0x0f] = 0;

              if (_index + *vx > 0xfff) {
                _register[0x0f] = 1;
              }
              
              _index += *vx;

              break;
            case 0x0029: // FX29 // sprites
              JDEBUG(JINFO, "FX29: %04x\n", opcode);

              _index = *vx * 0x05;

              break;
            case 0x0033: // FX33
              JDEBUG(JINFO, "FX33: %04x\n", opcode);

              _memory[_index + 0] = (*vx % 1000) / 100;
              _memory[_index + 1] = (*vx % 100) / 10;
              _memory[_index + 2] = (*vx % 10) / 1;

              break;
            case 0x0055: // FX55
              JDEBUG(JINFO, "FX55: %04x\n", opcode);

              for (int i=0; i<=x; i++) {
                _memory[_index + i] = _register[i];
              }

              break;
            case 0x0065: // FX65
              JDEBUG(JINFO, "FX65: %04x\n", opcode);

              for (int i=0; i<=x; i++) {
                _register[i] = _memory[_index + i];
              }

              break;
          }

          break;
        default:
          std::cout << "invalid opcode" << std::endl;
      }

      if (_delay_timer > 0) {
        _delay_timer--;
      }

      if (_sound_timer > 0) {
        _sound_timer--;
      }
    }

    bool LoadProgram(std::string path)
    {
      std::ifstream
        file(path, std::ios::binary);

      if (file.is_open() == false) {
        return false;
      }

      _program_counter = 0x200;

      uint8_t
        *ptr = _memory + _program_counter;
      char
        byte;

      std::cout << "loading program:" << std::endl;

      while (file.read(&byte, 1)) {
        *ptr++ = byte & 0xff;

        JDEBUG(JINFO, "%02x ", byte & 0xff);
      }
      
      std::cout << "\nloaded." << std::endl;

      Repaint();

      std::thread(
          [&]() {
          while (IsHidden() == false) {
            Process(); // process cpu instructions
          }
      }).detach();

      return true;
    }

		void Paint(jgui::Graphics *g) 
		{
      jgui::jsize_t<int>
        offset = (GetSize() - _screen)/2;

      jgui::BufferedImage
        buffer(jgui::JPF_RGB32, _screen);

      for (int j=0; j<_screen.height; j++) {
        for (int i=0; i<_screen.width; i++) {
          buffer.GetGraphics()->SetRawRGB((_video[j*_screen.width + i] == 0)?0xff000000:0xffffffff, {i, j});
        }
      }

      g->SetBlittingFlags(jgui::JBF_NEAREST);
      g->DrawImage(&buffer, {0, 0, GetSize()});
		}

    bool UpdateKey(jevent::jkeyevent_symbol_t key, int down)
    {
      if (key == jevent::JKS_ESCAPE) {
        exit(0);
      } else if (key == jevent::JKS_1) {
        _key[0x00] = down;
      } else if (key == jevent::JKS_2) {
        _key[0x01] = down;
      } else if (key == jevent::JKS_3) {
        _key[0x02] = down;
      } else if (key == jevent::JKS_4) {
        _key[0x03] = down;
      } else if (key == jevent::JKS_q) {
        _key[0x04] = down;
      } else if (key == jevent::JKS_w) {
        _key[0x05] = down;
      } else if (key == jevent::JKS_e) {
        _key[0x06] = down;
      } else if (key == jevent::JKS_r) {
        _key[0x07] = down;
      } else if (key == jevent::JKS_a) {
        _key[0x08] = down;
      } else if (key == jevent::JKS_s) {
        _key[0x09] = down;
      } else if (key == jevent::JKS_d) {
        _key[0x0a] = down;
      } else if (key == jevent::JKS_f) {
        _key[0x0b] = down;
      } else if (key == jevent::JKS_z) {
        _key[0x0c] = down;
      } else if (key == jevent::JKS_x) {
        _key[0x0d] = down;
      } else if (key == jevent::JKS_c) {
        _key[0x0e] = down;
      } else if (key == jevent::JKS_v) {
        _key[0x0f] = down;
      }

      return true;
    }

    virtual bool KeyPressed(jevent::KeyEvent *event)
    {
      return UpdateKey(event->GetSymbol(), 1);
    }

    virtual bool KeyReleased(jevent::KeyEvent *event)
    {
      return UpdateKey(event->GetSymbol(), 0);
    }
};

int main(int argc, char **argv)
{
	jgui::Application::Init(argc, argv);

	Chip8 app;

  if (app.LoadProgram(argv[1]) == false) {
    std::cout << "unable to load program" << std::endl;

    return -1;
  }

	app.SetTitle("Chip8");
  app.Exec();

	jgui::Application::Loop();

	return 0;
}


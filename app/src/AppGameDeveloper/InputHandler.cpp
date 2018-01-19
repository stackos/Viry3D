/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "InputHandler.h"
#include "CodeEditor.h"
#include "Input.h"
#include "math/Mathf.h"

namespace Viry3D
{
    struct InputHandlerKeyMap
    {
        KeyCode key;
        char c;
        char c_shift;
    };

    static InputHandlerKeyMap g_key_maps[] =
    {
        { KeyCode::BackQuote, '`', '~' },
        { KeyCode::Alpha1, '1', '!' },
        { KeyCode::Alpha2, '2', '@' },
        { KeyCode::Alpha3, '3', '#' },
        { KeyCode::Alpha4, '4', '$' },
        { KeyCode::Alpha5, '5', '%' },
        { KeyCode::Alpha6, '6', '^' },
        { KeyCode::Alpha7, '7', '&' },
        { KeyCode::Alpha8, '8', '*' },
        { KeyCode::Alpha9, '9', '(' },
        { KeyCode::Alpha0, '0', ')' },
        { KeyCode::Minus, '-', '_' },
        { KeyCode::Equals, '=', '+' },
        { KeyCode::LeftBracket, '[', '{' },
        { KeyCode::RightBracket, ']', '}' },
        { KeyCode::Backslash, '\\', '|' },
        { KeyCode::Semicolon, ';', ':' },
        { KeyCode::Quote, '\'', '"' },
        { KeyCode::Comma, ',', '<' },
        { KeyCode::Period, '.', '>' },
        { KeyCode::Slash, '/', '?' },
    };

    void InputHandler::OnKeyArrow(CodeEditor* editor)
    {
        const auto& lines = editor->GetLines();
        const auto& cursor_line = editor->GetCursorLine();

        if (Input::GetKeyDown(KeyCode::LeftArrow))
        {
            if (editor->GetCursorCharIndex() > 0)
            {
                editor->UpdateCursorPosition(cursor_line, editor->GetCursorCharIndex() - 1);
            }
            else if (editor->GetCursorCharIndex() == 0)
            {
                if (cursor_line != lines.begin())
                {
                    auto prev = cursor_line.Prev();
                    editor->UpdateCursorPosition(prev, -1);
                }
            }
            else if (editor->GetCursorCharIndex() == -1)
            {
                int text_size = (*cursor_line)->text.Size();
                if (text_size > 0)
                {
                    editor->UpdateCursorPosition(cursor_line, text_size - 1);
                }
                else
                {
                    if (cursor_line != lines.begin())
                    {
                        auto prev = cursor_line.Prev();
                        editor->UpdateCursorPosition(prev, -1);
                    }
                }
            }
        }
        else if (Input::GetKeyDown(KeyCode::RightArrow))
        {
            if (editor->GetCursorCharIndex() >= 0)
            {
                int text_size = (*cursor_line)->text.Size();
                if (editor->GetCursorCharIndex() < text_size - 1)
                {
                    editor->UpdateCursorPosition(cursor_line, editor->GetCursorCharIndex() + 1);
                }
                else
                {
                    editor->UpdateCursorPosition(cursor_line, -1);
                }
            }
            else if (editor->GetCursorCharIndex() == -1)
            {
                auto next = cursor_line.Next();
                if (next != lines.end())
                {
                    if ((*next)->text.Size() > 0)
                    {
                        editor->UpdateCursorPosition(next, 0);
                    }
                    else
                    {
                        editor->UpdateCursorPosition(next, -1);
                    }
                }
            }
        }
        else if (Input::GetKeyDown(KeyCode::UpArrow))
        {
            if (cursor_line != lines.begin())
            {
                auto prev = cursor_line.Prev();
                if (editor->GetCursorCharIndex() >= 0)
                {
                    if (editor->GetCursorCharIndex() <= (*prev)->text.Size() - 1)
                    {
                        editor->UpdateCursorPosition(prev, editor->GetCursorCharIndex());
                    }
                    else
                    {
                        editor->UpdateCursorPosition(prev, -1);
                    }
                }
                else if (editor->GetCursorCharIndex() == -1)
                {
                    if ((*cursor_line)->text.Size() < (*prev)->text.Size())
                    {
                        editor->UpdateCursorPosition(prev, (*cursor_line)->text.Size());
                    }
                    else
                    {
                        editor->UpdateCursorPosition(prev, -1);
                    }
                }
            }
        }
        else if (Input::GetKeyDown(KeyCode::DownArrow))
        {
            auto next = cursor_line.Next();
            if (next != lines.end())
            {
                if (editor->GetCursorCharIndex() >= 0)
                {
                    if (editor->GetCursorCharIndex() <= (*next)->text.Size() - 1)
                    {
                        editor->UpdateCursorPosition(next, editor->GetCursorCharIndex());
                    }
                    else
                    {
                        editor->UpdateCursorPosition(next, -1);
                    }
                }
                else if (editor->GetCursorCharIndex() == -1)
                {
                    if ((*cursor_line)->text.Size() < (*next)->text.Size())
                    {
                        editor->UpdateCursorPosition(next, (*cursor_line)->text.Size());
                    }
                    else
                    {
                        editor->UpdateCursorPosition(next, -1);
                    }
                }
            }
        }
    }

    void InputHandler::HandleKeyEvents(CodeEditor* editor)
    {
        if (Input::GetKey(KeyCode::LeftAlt) ||
            Input::GetKey(KeyCode::RightAlt))
        {
            return;
        }

        bool ctrl = false;
        bool shift = false;
        bool caps_on = Input::GetKey(KeyCode::CapsLock);

        if (Input::GetKey(KeyCode::LeftControl) ||
            Input::GetKey(KeyCode::RightControl))
        {
            ctrl = true;
        }

        if (Input::GetKey(KeyCode::LeftShift) ||
            Input::GetKey(KeyCode::RightShift))
        {
            shift = true;
        }

        const auto& lines = editor->GetLines();
        const auto& cursor_line = editor->GetCursorLine();

        if (!ctrl)
        {
            if (!shift)
            {
                if (Input::GetKeyDown(KeyCode::LeftArrow) ||
                    Input::GetKeyDown(KeyCode::RightArrow) ||
                    Input::GetKeyDown(KeyCode::UpArrow) ||
                    Input::GetKeyDown(KeyCode::DownArrow))
                {
                    this->OnKeyArrow(editor);
                }
                else if (Input::GetKeyDown(KeyCode::Return))
                {
                    editor->InsertLine();
                }
                else if (Input::GetKeyDown(KeyCode::Backspace))
                {
                    editor->RemoveChar();
                }
                else if (Input::GetKeyDown(KeyCode::Tab))
                {
                    int space_count;
                    int char_index = editor->GetCursorCharIndex();
                    if (char_index >= 0)
                    {
                        space_count = 4 - char_index % 4;
                    }
                    else
                    {
                        space_count = 4 - (*editor->GetCursorLine())->text.Size() % 4;
                    }
                    Vector<char> spaces(space_count, ' ');
                    editor->InsertString(String(&spaces[0], space_count));
                }
                else if (Input::GetKeyDown(KeyCode::Space))
                {
                    editor->InsertString(" ");
                }
            }
            
            for (int i = (int) KeyCode::A; i <= (int) KeyCode::Z; ++i)
            {
                if (Input::GetKeyDown((KeyCode) i))
                {
                    char c;
                    if ((caps_on && !shift) ||
                        (!caps_on && shift))
                    {
                        c = 'A' + i - (int) KeyCode::A;
                    }
                    else
                    {
                        c = 'a' + i - (int) KeyCode::A;
                    }
                    
                    editor->InsertString(String(&c, 1));
                }
            }

            int key_map_size = sizeof(g_key_maps) / sizeof(g_key_maps[0]);
            for (int i = 0; i <= key_map_size; ++i)
            {
                if (Input::GetKeyDown(g_key_maps[i].key))
                {
                    char c;
                    if ((caps_on && !shift) ||
                        (!caps_on && shift))
                    {
                        c = g_key_maps[i].c_shift;
                    }
                    else
                    {
                        c = g_key_maps[i].c;
                    }

                    editor->InsertString(String(&c, 1));
                }
            }
        }
        else
        {
            // ctrl + c
            // ctrl + v
        }

        if (!Mathf::FloatEqual(Input::GetMouseScrollWheel(), 0))
        {
            editor->ScrollLine((int) Input::GetMouseScrollWheel() * 3);
        }
    }
}

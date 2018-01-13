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

namespace Viry3D
{
    void InputHandler::HandleInput(CodeEditor* editor)
    {
        if (Input::GetKey(KeyCode::LeftAlt) ||
            Input::GetKey(KeyCode::RightAlt))
        {
            return;
        }

        bool ctrl = false;
        bool shift = false;

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
            else if (Input::GetKeyDown(KeyCode::Return))
            {
                editor->InsertLine();
            }
            else if (Input::GetKeyDown(KeyCode::Backspace))
            {

            }
        }
        else
        {
            // ctrl + c
            // ctrl + v
        }
    }
}

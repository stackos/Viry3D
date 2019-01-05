/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
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

#pragma once

#include "Demo.h"
#include "Application.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "time/Time.h"
#include "ui/CanvasRenderer.h"
#include "ui/Label.h"
#include "ui/Font.h"
#include "ui/LineView.h"
#include "2d/NavigationPolygon.h"
#include "2d/Navigation2D.h"

namespace Viry3D
{
    class DemoNavigation2D : public Demo
    {
    public:
        Camera* m_ui_camera = nullptr;
        Label* m_label = nullptr;
        CanvasRenderer* m_canvas = nullptr;
        Ref<NavigationPolygonInstance> m_navigation;

        void InitNavigation()
        {
            std::vector<double> vertices_values = { 587.833, 271.924, 530.464, 284.878, 508.256, 281.177, 497.153, 255.269, 624.926, 359.595, 648.903, 394.065, 620.443, 383.995, 669.26, 297.833, 648.903, 321.891, 650.754, 251.567, 619.293, 510.654, 676.663, 493.998, 706.272, 501.401, 669.26, 529.16, 602.638, 523.608, 587.833, 179.393, 573.028, 140.53, 645.202, 159.036, 710.106, 179.216, 630.397, 212.704, 597.086, 192.348, 471.244, 251.567, 421.277, 270.074, 428.68, 246.015, 502.704, 97.9661, 517.509, 55.4019, 537.866, 99.8167, 536.016, 175.692, 495.302, 164.588, 487.899, 85.0117, 310.24, 75.7586, 308.39, 92.4142, 345.402, 210.854, 360.207, 223.808, 297.286, 258.97, 288.033, 231.211, 319.493, 190.497, 193.651, 423.675, 245.469,477.343, 221.41, 488.446, 147.386, 408.87, 182.548, 382.961, 145.584, 224.311, 175.145, 332.995, 202.904, 99.8167, 310.24, 62.8043, 695.169, 303.385, 682.214, 284.878,
                598.937, 492.148, 571.177, 501.401, 605.437, 456.366, 621.144, 486.596, 538.077, 499.891, 395.879, 501.87, 536.407, 524.944, 371.311, 518.056, 573.028, 94.2648, 582.281, 47.9994, 667.409, 75.7586, 350.954, 447.733, 363.908, 351.501, 384.265, 351.501, 376.862, 418.123, 373.441, 436.494, 424.978, 334.845, 421.277, 360.754, 352.804, 320.04, 321.344, 338.546, 299.136, 283.028, 241.767, 327.443, 234.365, 244.165, 325.228, 486.302, 300.441, 497.494, 317.643, 447.733, 332.441, 457.494, 524.608, 359.37, 526.762, 342.248, 366.441, 467.494, 480.497, 434.779, 496.638, 439.381, 476.441, 468.494, 265.825, 407.019, 184.398, 349.65, 310.24, 112.771, 267.676, 153.485, 221.41, 171.991, 700.721, 268.223, 397.219, 188.646, 415.725, 177.543, 465.692, 179.393, 476.796, 207.152, 443.485, 192.348, 437.933, 170.14, 452.738, 166.439, 460.14, 123.875, 476.796, 149.783, 189.95, 231.211 };
            std::vector<std::vector<int>> polygons = {
                { 0, 1, 2, 3 },
                { 4, 5, 6 },
                { 7, 8, 9 },
                { 10, 11, 12, 13, 14 },
                { 15, 16, 17, 18, 19, 20 },
                { 21, 3, 2, 22, 23 },
                { 24, 25, 26, 27, 28 },
                { 25, 24, 29 },
                { 30, 25, 29, 31 },
                { 32, 33, 34, 35, 36 },
                { 37, 38, 39, 40 },
                { 41, 37, 40 },
                { 41, 40, 42, 43 },
                { 44, 45, 30, 31 },
                { 46, 12, 11, 7, 47 },
                { 47, 7, 9 },
                { 48, 10, 14, 49 },
                { 50, 6, 5, 51, 48 },
                { 52, 50, 48, 49 },
                { 53, 52, 49, 54, 55 },
                { 17, 56, 57, 58, 18 },
                { 59, 60, 61, 62, 63 },
                { 64, 65, 61, 66 },
                { 66, 61, 60, 67, 68 },
                { 68, 67, 69, 70 },
                { 68, 70, 35, 34 },
                { 71, 53, 55, 72 },
                { 71, 72, 73, 74 },
                { 4, 6, 75, 76 },
                { 63, 77, 74, 59 },
                { 78, 2, 1, 76, 75, 79, 80 },
                { 78, 80, 63, 62 },
                { 81, 59, 74, 73 },
                { 81, 73, 41, 82 },
                { 44, 31, 83, 84, 85 },
                { 18, 86, 47, 9, 19 },
                { 15, 20, 3, 21 },
                { 23, 22, 87, 88 },
                { 89, 28, 27, 90, 91 },
                { 89, 91, 92, 93 },
                { 36, 94, 95, 93, 92 },
                { 36, 92, 88 },
                { 36, 88, 87, 32 },
                { 36, 35, 85, 84 },
                { 42, 44, 85, 96 },
                { 42, 96, 43 },
                { 41, 43, 82 }
            };
            std::vector<std::vector<double>> outlines_values = {
                { 221.41, 488.446, 147.386, 408.87, 145.584, 224.311, 202.904, 99.8167, 310.24, 62.8043, 310.24, 75.7586, 517.509, 55.4019, 537.866, 99.8167, 536.016, 175.692, 476.796, 207.152, 443.485, 192.348, 437.933, 170.14, 415.725, 177.543, 428.68, 246.015, 471.244, 251.567, 587.833, 179.393, 573.028, 140.53, 645.202, 159.036, 573.028, 94.2648, 582.281, 47.9994, 667.409, 75.7586, 710.106, 179.216, 700.721, 268.223, 682.214, 284.878, 695.169, 303.385, 706.272, 501.401, 669.26, 529.16, 602.638, 523.608, 571.177, 501.401, 536.407, 524.944, 371.311, 518.056, 300.441, 497.494, 317.643, 447.733, 182.548, 382.961, 193.651, 423.675, 245.469, 477.343 },
                { 350.954, 447.733, 363.908, 351.501, 321.344, 338.546, 241.767, 327.443, 234.365, 244.165, 288.033, 231.211, 221.41, 171.991, 189.95, 231.211, 175.145, 332.995, 184.398, 349.65, 265.825, 407.019 },
                { 267.676, 153.485, 310.24, 112.771, 308.39, 92.4142, 487.899, 85.0117, 502.704, 97.9661, 495.302, 164.588, 465.692, 179.393, 452.738, 166.439, 476.796, 149.783, 460.14, 123.875, 319.493, 190.497 },
                { 397.219, 188.646, 345.402, 210.854, 360.207, 223.808, 297.286, 258.97, 299.136, 283.028, 352.804, 320.04, 424.978, 334.845, 421.277, 360.754, 384.265, 351.501, 376.862, 418.123, 480.497, 434.779, 508.256, 281.177, 421.277, 270.074 },
                { 497.153, 255.269, 597.086, 192.348, 630.397, 212.704, 650.754, 251.567, 648.903, 321.891, 669.26, 297.833, 676.663, 493.998, 619.293, 510.654, 598.937, 492.148, 621.144, 486.596, 648.903, 394.065, 624.926, 359.595, 526.762, 342.248, 530.464, 284.878, 587.833, 271.924 },
                { 325.228, 486.302, 332.441, 457.494, 366.441, 467.494, 373.441, 436.494, 476.441, 468.494, 496.638, 439.381, 524.608, 359.37, 620.443, 383.995, 605.437, 456.366, 538.077, 499.891, 395.879, 501.87 }
            };

            std::vector<Vector2> vertices(vertices_values.size() / 2);
            for (size_t i = 0; i < vertices.size(); ++i)
            {
                vertices[i] = Vector2((float) vertices_values[i * 2 + 0], (float) vertices_values[i * 2 + 1]);
            }

            std::vector<std::vector<Vector2>> outlines(outlines_values.size());
            for (size_t i = 0; i < outlines.size(); ++i)
            {
                outlines[i].resize(outlines_values[i].size() / 2);
                for (size_t j = 0; j < outlines[i].size(); ++j)
                {
                    outlines[i][j] = Vector2((float) outlines_values[i][j * 2 + 0], (float) outlines_values[i][j * 2 + 1]);
                }
            }

            auto poly = RefMake<NavigationPolygon>();
            poly->SetVertices(vertices);
            for (size_t i = 0; i < polygons.size(); ++i)
            {
                poly->AddPolygon(polygons[i]);
            }
            for (size_t i = 0; i < outlines.size(); ++i)
            {
                poly->AddOutline(outlines[i]);
            }

            m_navigation = RefMake<NavigationPolygonInstance>();
            m_navigation->SetNavigationPolygon(poly);
            auto nav2d = m_navigation->GetNavigation2D();

            // line view
            Vector<Vector2i> lines;

            auto poly_lines = RefMake<LineView>();
            poly_lines->SetSize(Vector2i(VIEW_SIZE_FILL_PARENT, VIEW_SIZE_FILL_PARENT));
            poly_lines->SetColor(Color(1, 1, 1, 1));
            poly_lines->SetLineWidth(1);
            lines.Clear();
            for (size_t i = 0; i < polygons.size(); ++i)
            {
                for (size_t j = 0; j < polygons[i].size(); ++j)
                {
                    int begin_index = polygons[i][j];
                    int end_index;
                    if (j == polygons[i].size() - 1)
                    {
                        end_index = polygons[i][0];
                    }
                    else
                    {
                        end_index = polygons[i][j + 1];
                    }
                    
                    lines.Add(Vector2i((int) vertices[begin_index].x, (int) vertices[begin_index].y));
                    lines.Add(Vector2i((int) vertices[end_index].x, (int) vertices[end_index].y));
                }
            }
            poly_lines->SetLines(lines);
            m_canvas->AddView(poly_lines);

            auto out_lines = RefMake<LineView>();
            out_lines->SetSize(Vector2i(VIEW_SIZE_FILL_PARENT, VIEW_SIZE_FILL_PARENT));
            out_lines->SetColor(Color(1, 0, 0, 1));
            out_lines->SetLineWidth(3);
            lines.Clear();
            for (size_t i = 0; i < outlines.size(); ++i)
            {
                for (size_t j = 0; j < outlines[i].size(); ++j)
                {
                    Vector2 begin = outlines[i][j];
                    Vector2 end;
                    if (j == outlines[i].size() - 1)
                    {
                        end = outlines[i][0];
                    }
                    else
                    {
                        end = outlines[i][j + 1];
                    }

                    lines.Add(Vector2i((int) begin.x, (int) begin.y));
                    lines.Add(Vector2i((int) end.x, (int) end.y));
                }
            }
            out_lines->SetLines(lines);
            m_canvas->AddView(out_lines);

            // find path
            std::vector<Vector2> path = nav2d->GetSimplePath(Vector2(640, 100), Vector2(160, 370), true);
            if (path.size() > 1)
            {
                // show path
                auto path_lines = RefMake<LineView>();
                path_lines->SetSize(Vector2i(VIEW_SIZE_FILL_PARENT, VIEW_SIZE_FILL_PARENT));
                path_lines->SetColor(Color(0, 1, 0, 1));
                path_lines->SetLineWidth(2);
                lines.Clear();
                for (size_t i = 0; i < path.size() - 1; ++i)
                {
                    Vector2 begin = path[i];
                    Vector2 end = path[i + 1];

                    lines.Add(Vector2i((int) begin.x, (int) begin.y));
                    lines.Add(Vector2i((int) end.x, (int) end.y));
                }
                path_lines->SetLines(lines);
                m_canvas->AddView(path_lines);
            }
        }

        void InitUI()
        {
            m_ui_camera = Display::Instance()->CreateCamera();

            auto canvas = RefMake<CanvasRenderer>();
            m_ui_camera->AddRenderer(canvas);

            auto label = RefMake<Label>();
            canvas->AddView(label);

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, 40));
            label->SetFont(Font::GetFont(FontType::Consola));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);

            m_label = label.get();
            m_canvas = canvas.get();
        }

        virtual void Init()
        {
            this->InitUI();
            this->InitNavigation();
        }

        virtual void Done()
        {
            if (m_ui_camera)
            {
                Display::Instance()->DestroyCamera(m_ui_camera);
                m_ui_camera = nullptr;
            }
        }

        virtual void Update()
        {
            if (m_label)
            {
                m_label->SetText(String::Format("FPS:%d", Time::GetFPS()));
            }
        }
    };
}

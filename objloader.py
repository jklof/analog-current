from tft2 import DisplayController
import re

class OBJLoader:
    def __init__(self, file_path):
        self.objects = {}  # Dictionary to store multiple objects
        self.current_object = None  # Name of the current object
        self.vertices = [] # vertices are common for all
        self.normals = []  # normals are common for all
        self.load(file_path)

    def load(self, file_path):
        with open(file_path, 'r') as file:
            for line in file:
                line = line.strip()
                if not line:
                    continue

                elements = line.split()
                if elements[0] == 'o':
                    # New object definition
                    object_name = elements[1]
                    self.current_object = object_name
                    self.objects[self.current_object] = { 'faces': [], 'lines': []}
                elif elements[0] == 'v':
                    # Vertex coordinates
                    vertex = [float(elements[1]), float(elements[2]), float(elements[3])]
                    self.vertices.append(vertex)
                elif elements[0] == 'vn':
                    # Vertex normals
                    normal = [float(elements[1]), float(elements[2]), float(elements[3])]
                    self.normals.append(normal)
                elif elements[0] == 'f':
                    # Faces (vertex indices and normals indices)
                    face = []
                    for elem in elements[1:]:
                        vertex_indices = elem.split('/')
                        vertex_index = int(vertex_indices[0]) - 1  # Subtract 1 as OBJ indices start from 1
                        normal_index = int(vertex_indices[2]) - 1  # Subtract 1 for the same reason
                        face.append((vertex_index, normal_index))
                    self.objects[self.current_object]['faces'].append(face)
                elif elements[0] == 'l':
                    # Line segments (vertex indices)
                    line_segment = [int(elem) - 1 for elem in elements[1:]]
                    self.objects[self.current_object]['lines'].append(line_segment)



    def print_as_c_header(self, output_file, target_max_value=127):

        # Find the maximum absolute value among vertices
        max_abs_value = 0
        for v in self.vertices:
            max_abs_value = max(max_abs_value, abs(v[0]), abs(v[2]))
        # Calculate the scale factor to fit the maximum value into target_max_value
        scale = target_max_value / max_abs_value

        # Open the output file for writing
        with open(output_file, 'w') as file:

            file.write("const int8_t vertices[] PROGMEM = {\n")
            for v in self.vertices:
                x = int(v[0] * scale)
                y = int(v[2] * scale)
                file.write( f"{x}, {y},\n")

            file.write("};\n")

            for object_name, object_data in self.objects.items():
                clean_label = clean_name = re.sub(r'[^A-Za-z0-9]+', '_', object_name)
                file.write("const uint16_t {0}_Lines[] PROGMEM = {{".format(clean_label))

                adv = int( self.horiz_advance(object_name) * scale )

                file.write(f"/* adv*/ {adv}, /*vi*/ ")


                for line_segment in object_data['lines']:
                    line_str = ", ".join(str(idx) for idx in line_segment)
                    file.write("{0}, ".format(line_str))
                file.write("};\n")


    def boundingbox(self, object_name):
        lines = self.objects[object_name]['lines']
        min_x = min(self.vertices[line[0]][0] for line in lines)
        max_x = max(self.vertices[line[0]][0] for line in lines)
        #z is not used
        #min_z = min(self.vertices[line[0]][1] for line in lines)
        #max_z = max(self.vertices[line[0]][1] for line in lines)
        min_y = min(self.vertices[line[0]][2] for line in lines)
        max_y = max(self.vertices[line[0]][2] for line in lines)
        return min_x, min_y, max_x, max_y

    def horiz_advance(self, object_name):
        lines = self.objects[object_name]['lines']
        return max(self.vertices[line[0]][0] for line in lines)

    def drawlines(self, dc, object, x,y,scale):
        lines = self.objects[object]['lines']  # Retrieve the line segments from the object
        for line_segment in lines:
            # z is not used
            x0, _, y0 = self.vertices[line_segment[0]]  # Get the coordinates of the first vertex
            x1, _, y1 = self.vertices[line_segment[1]]  # Get the coordinates of the second vertex
            # Draw the line segment
            dc.line(int(x + x0 * scale), int(y + y0 * scale), int(x + x1 * scale), int(y + y1 * scale))

    def print(self, dc, text, x,y, scale):
        char_map = {
            '.' : 'Letter__dot',
            '+' : 'Letter__plus',
            '-' : 'Letter__minus',
            ':' : 'Letter__colon',
            '=' : 'Letter__equals',
            '/' : 'Letter__slash',
            '#' : 'Letter__hash',
            '*' : 'Letter__asterisk',
            '$' : 'Letter__kwh',
            '0' : 'Letter_0',
            '1' : 'Letter_1',
            '2' : 'Letter_2',
            '3' : 'Letter_3',
            '4' : 'Letter_4',
            '5' : 'Letter_5',
            '6' : 'Letter_6',
            '7' : 'Letter_7',
            '8' : 'Letter_8',
            '9' : 'Letter_9',
            'A' : 'Letter_A',
            'B' : 'Letter_B',
            'C' : 'Letter_C',
            'D' : 'Letter_D',
            'E' : 'Letter_E',
            'F' : 'Letter_F',
            'G' : 'Letter_G',
            'H' : 'Letter_H',
            'I' : 'Letter_I',
            'J' : 'Letter_J',
            'K' : 'Letter_K',
            'L' : 'Letter_L',
            'M' : 'Letter_M',
            'N' : 'Letter_N',
            'O' : 'Letter_O',
            'P' : 'Letter_P',
            'Q' : 'Letter_Q',
            'R' : 'Letter_R',
            'S' : 'Letter_S',
            'T' : 'Letter_T',
            'U' : 'Letter_U',
            'V' : 'Letter_V',
            'W' : 'Letter_W',
            'X' : 'Letter_X',
            'Y' : 'Letter_Y',
            'Z' : 'Letter_Z',
        }
        for char in text:
            if char == ' ': # specially handle space
                x += self.horiz_advance('Letter_X') * scale
            else:
                char_object = char_map.get(char, 'Letter_dot')  # Default to dot
                self.drawlines(dc, char_object, x,y, scale)
                x += self.horiz_advance(char_object) * scale





if __name__ == '__main__':

    vfont = OBJLoader("./vfont.obj")

    # no longer needed, use blender_fontmaker.py
    #vfont.print_as_c_header('./tft_display/vfont.h')

    with DisplayController('COM7') as dc:
        dc.sync()
        dc.color(0xff,0xff,0xff)
        dc.clear()
        vfont.print(dc, "*.-+BAUHAUS 93 FONT+-.*", 0, 30, 30)
        vfont.print(dc, '0123456789.+-:', 0,100,60)
        vfont.print(dc, 'ABCDEFGHIJKLMN', 0,160,60)
        vfont.print(dc, 'OPQRSTUVWXYZ', 0,220,60)
        vfont.print(dc, '$', 0,280,60)


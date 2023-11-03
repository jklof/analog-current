from tft2 import DisplayController

class OBJLoader:
    def __init__(self, file_path):
        self.objects = {}  # Dictionary to store multiple objects
        self.current_object = None  # Name of the current object
        self.vertices = [] # vertices are common for all..
        self.normals = []  # normals are common for all?
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
            '0' : 'Number.0',
            '1' : 'Number.1',
            '2' : 'Number.2',
            '3' : 'Number.3',
            '4' : 'Number.4',
            '5' : 'Number.5',
            '6' : 'Number.6',
            '7' : 'Number.7',
            '8' : 'Number.8',
            '9' : 'Number.9',
            '.' : 'Number.dot',
            '+' : 'Number.plus',
            '-' : 'Number.minus',
            ':' : 'Number.dot2',
            'A' : 'Letter.A',
            'B' : 'Letter.B',
            'C' : 'Letter.C',
            'D' : 'Letter.D',
            'E' : 'Letter.E',
            'F' : 'Letter.F',
            'G' : 'Letter.G',
            'H' : 'Letter.H',
            'I' : 'Letter.I',
            'J' : 'Letter.J',
            'K' : 'Letter.K',
            'L' : 'Letter.L',
            'M' : 'Letter.M',
            'N' : 'Letter.N',
            'O' : 'Letter.O',
            'P' : 'Letter.P',
            'Q' : 'Letter.Q',
            'R' : 'Letter.R',
            'S' : 'Letter.S',
            'T' : 'Letter.T',
            'U' : 'Letter.U',
            'V' : 'Letter.V',
            'W' : 'Letter.W',
            'X' : 'Letter.X',
            'Y' : 'Letter.Y',
            'Z' : 'Letter.Z',
            '¤' : 'Symbol.ce_kwh'
        }
        for char in text:
            if char == ' ': # specially handle space
                x += self.horiz_advance('Letter.X') * scale
            else:
                char_object = char_map.get(char, 'Number.dot')  # Default to dot
                self.drawlines(dc, char_object, x,y, scale)
                x += self.horiz_advance(char_object) * scale





if __name__ == '__main__':

    vfont = OBJLoader("./vfont.obj")
    with DisplayController('COM7') as dc:
        dc.sync()
        dc.console(False)
        dc.clear()
        vfont.print(dc, "..-+BAUHAUS 93 FONT+-..", 0, 30, 30)
        vfont.print(dc, '0123456789.+-:', 0,100,60)
        vfont.print(dc, 'ABCDEFGHIJKLMN', 0,160,60)
        vfont.print(dc, 'OPQRSTUVWXYZ', 0,220,60)
        vfont.print(dc, '¤', 0,280,60)

import cv2
import numpy as np
from os import listdir
from os.path import isfile, join


path = "C:/don-t_forget_kids_in_car/server/img/"

def getFileList():
    global path
    return [f for f in listdir(path) if isfile(join(path, f))]

def mouseCB(event, x, y, flags, param):
    print(x, ", ", y)

def subtraction(prev_frame, current_frame):
    #cv2.imshow('prev_frame', prev_frame)

    prev_gray = cv2.cvtColor(prev_frame, cv2.COLOR_BGR2GRAY)
    prev_gray = cv2.GaussianBlur(prev_gray, (21, 21), 0)
    

    gray = cv2.cvtColor(current_frame, cv2.COLOR_BGR2GRAY)
    gray = cv2.GaussianBlur(gray, (21, 21), 0)

    xMin = 200
    xMax = 1030
    h = current_frame.shape[0]
    w = current_frame.shape[1]

    diff_frame = cv2.absdiff(prev_gray, gray)
    points = np.array([[0, 0], [xMin, 0], [xMin, h], [0, h]])
    cv2.fillPoly(diff_frame, pts=[points], color=(0, 0, 0))
    points = np.array([[xMax, 0], [w, 0], [w, h], [xMax, h]])
    cv2.fillPoly(diff_frame, pts=[points], color=(0, 0, 0))

    thresh_frame = cv2.threshold(diff_frame, 30, 255, cv2.THRESH_BINARY)[1]

    # Taking a matrix of size 5 as the kernel
    kernel = np.ones((5, 5), np.uint8)
    thresh_frame = cv2.dilate(thresh_frame, kernel, iterations = 2)
    cnts,_ = cv2.findContours(thresh_frame.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    current_frame = cv2.line(current_frame, (xMin, 0), (xMin, h), (255, 0, 255), 2)
    current_frame = cv2.line(current_frame, (xMax, 0), (xMax, h), (255, 0, 255), 2)

    diff_rgb = cv2.cvtColor(diff_frame,cv2.COLOR_GRAY2RGB)
    thresh_rgb = cv2.cvtColor(thresh_frame,cv2.COLOR_GRAY2RGB)

    for indx, contour in enumerate(cnts):
        if cv2.contourArea(contour) < 5000:
            cv2.drawContours(image=current_frame, contours=cnts, contourIdx=indx, color=(0, 255, 0), thickness=1, lineType=cv2.LINE_AA)
            #continue
        else:
            cv2.drawContours(image=current_frame, contours=cnts, contourIdx=indx, color=(0, 0, 255), thickness=3, lineType=cv2.LINE_AA)
            cv2.drawContours(image=diff_rgb, contours=cnts, contourIdx=indx, color=(0, 0, 255), thickness=1, lineType=cv2.LINE_AA)
            cv2.drawContours(image=thresh_rgb, contours=cnts, contourIdx=indx, color=(0, 0, 255), thickness=cv2.FILLED)

        '''
        (x, y, w, h) = cv2.boundingRect(contour)
        if x > xMin and x+w < xMax:
            # making green rectangle around the moving object
            cv2.rectangle(current_frame, (x, y), (x + w, y + h), (0, 255, 0), 1)
        '''


    vis1 = np.concatenate((prev_frame, current_frame), axis=1)
    vis2 = np.concatenate((diff_rgb, thresh_rgb), axis=1)
    vis = np.concatenate((vis1, vis2), axis=0)
    #cv2.imshow('Motion Frame', resize_percentage(vis, 0.4))
    #cv2.imshow('Diff Frame', diff_frame)
    #cv2.imshow('Thres Frame', thresh_frame)
    #cv2.imshow('Result', current_frame)
    #cv2.setMouseCallback('Result',mouseCB)
    #key = cv2.waitKey(0) & 0xFF
    key=1
    return key, vis, current_frame

def resize_percentage(img, scale):
    width = int(img.shape[1] * scale)
    height = int(img.shape[0] * scale)
    dim = (width, height)
      
    # resize image
    resized = cv2.resize(img, dim, interpolation = cv2.INTER_AREA)
    return resized

def main():
    global path
    print("ABC")
    files = getFileList()
    print(files)

    img_arr = []
    sin_arr = []
    for i in range(1, len(files)):
        print(files[i-1], " : ", files[i])
        prev_frame = cv2.imread(path+files[i-1])
        current_frame = cv2.imread(path+files[i])
        key, res, single = subtraction(prev_frame, current_frame)
        img_arr.append(res)
        sin_arr.append(single)
        
        #cv2.imwrite('111'+'_result_'+files[i], res)
        if key == ord('q'):
            break
    createVdo(img_arr, 'combine.mpeg')
    createVdo(sin_arr, 'single.mpeg')

def createVdo(img_arr, fname):
    h, w, layer = img_arr[0].shape
    size = (w, h)
    out = cv2.VideoWriter(fname, cv2.VideoWriter_fourcc(*'DIVX'), 2, size)
    
    n = len(img_arr)
    for i in range(n):
        print(i, "/", n)
        out.write(img_arr[i])
    out.release()

if __name__ == '__main__':
    main()

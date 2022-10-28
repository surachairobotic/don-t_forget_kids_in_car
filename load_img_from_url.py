from PIL import Image
import requests, cv2, time, copy, imutils
from io import BytesIO
import numpy as np

def main():
    base_frame = None
    bSaveBase = False

    while True:
        t = time.time()
        response = requests.get('http://192.168.1.106/capture')
        img = Image.open(BytesIO(response.content))

        #img.show()

        pil_image = img.convert('RGB') 
        open_cv_image = np.array(pil_image)
        # Convert RGB to BGR 
        open_cv_image = open_cv_image[:, :, ::-1].copy()
        (h, w) = open_cv_image.shape[:2]
        div = 1
        dim = (int(w/div), int(h/div))
        open_cv_image = cv2.resize(open_cv_image, dim)
        
        cv2.imshow('ESP32CAM', open_cv_image)

        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'): # wait for 1 millisecond
            break
        elif key == ord('s'):
            gray = cv2.cvtColor(open_cv_image, cv2.COLOR_BGR2GRAY)
            base_frame = copy.deepcopy(gray)
            bSaveBase = True

        if bSaveBase:
            cv2.imshow('Base Frame', base_frame)

            gray = cv2.cvtColor(open_cv_image, cv2.COLOR_BGR2GRAY)
            gray = cv2.GaussianBlur(gray, (21, 21), 0)
            diff_frame = cv2.absdiff(base_frame, gray)
            thresh_frame = cv2.threshold(diff_frame, 30, 255, cv2.THRESH_BINARY)[1]
            thresh_frame = cv2.dilate(thresh_frame, None, iterations = 2)
            cnts,_ = cv2.findContours(thresh_frame.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

            for contour in cnts:
                if cv2.contourArea(contour) < 100:
                    continue
                motion = 1
        
                (x, y, w, h) = cv2.boundingRect(contour)
                # making green rectangle around the moving object
                cv2.rectangle(open_cv_image, (x, y), (x + w, y + h), (0, 255, 0), 3)

            cv2.imshow('Diff Frame', diff_frame)
            cv2.imshow('Thres Frame', thresh_frame)
            cv2.imshow('Result', open_cv_image)

        dt = time.time()-t
        print("FPS : {}".format(1.0/dt))

if __name__ == "__main__":
    main()
//
// Created by root on 17-8-30.
//
#include "cl_common.h"
#include "mtcnn.h"

using namespace cv;
using namespace std;
using namespace cl::fd;
using namespace cl::fa;
namespace fs = boost::filesystem;


static std::vector<std::string> get_file_list(const std::string& path)
{
    vector<string> m_file_list;
    if (!path.empty())
    {
        fs::path apk_path(path);
        fs::recursive_directory_iterator end;

        for (fs::recursive_directory_iterator i(apk_path); i != end; ++i)
        {
            if (fs::is_regular_file(i->path())){
                const fs::path cp = (*i);
                m_file_list.push_back(cp.string());
            }
        }
    }
    return m_file_list;
}



int main(int argc, char** argv) {
    /*must change these there lines to yours**/
    const string model_dir = "/home/tpys/projects/cl-face/trained_models/detection"; // mtcnn model dir
    const string dataset_dir = "/home/tpys/dataset/lfw"; //dataset need to be align
    const string output_dir = "/home/tpys/dataset/lfw_align_112x96";//output aligned dataset

    bool verbose = false;//turn on to visulize align result
    const vector<string> argvs = {
            model_dir + "/det1.prototxt",
            model_dir + "/det2.prototxt",
            model_dir + "/det3.prototxt",
            model_dir + "/det1.caffemodel",
            model_dir + "/det2.caffemodel",
            model_dir + "/det3.caffemodel",
    };

    std::shared_ptr<Alignment> face_alignment = std::make_shared<mtcnn::fd::FaceDetector>();
    if(!face_alignment->load_model({argvs[0], argvs[1], argvs[2]}, {argvs[3], argvs[4], argvs[5]}))
    {
        cout <<"Can't load face detection model!" << endl;
        return  -1;
    }
    auto image_list = get_file_list(dataset_dir);
    int64 total_num = image_list.size();
    fs::path dst_p(output_dir);
    int64 counter = 0;


    for(auto& e: image_list){
        fs::path src_p(e);
        auto dir_path = dst_p / src_p.parent_path().filename();
        if(!fs::exists(dir_path)){
            fs::create_directories(dir_path);
        }

        Mat src = imread(e);
        if (src.empty()) continue;

        Mat src_gray;
        Mat aligned_face;
        Mat src_display = src.clone();
        if(src.channels() == 3)
        {
            cv::cvtColor(src, src_gray, CV_BGR2GRAY);
        }
        else
        {
            src_gray = src.clone();
        }
        vector<cl::FaceBox> face_boxes;
        vector<cl::FaceLandmark> face_landmarks;
        face_alignment->detect(src, face_boxes, face_landmarks);

        if(face_landmarks.size() > 0){
            auto biggest_id = cl::fa::get_biggest_id(face_landmarks, src.cols, src.rows);
            auto aligned_face = face_alignment->align_face(src, face_landmarks[biggest_id], 96, 112);

            auto full_name = dir_path/src_p.filename();
            imwrite(full_name.string(), aligned_face);
            ++counter;

            char msg[256];
            if(counter % 1000 == 0){
                sprintf(msg, "process: %d/%d", counter, total_num);
                cout << msg << endl;
            }


            if(verbose){
                cl::fa::draw(face_boxes[biggest_id], face_landmarks[biggest_id], src_display);
                imshow("aligned", aligned_face);
                imshow("src", src_display);
                waitKey(0);
            }
        }
    }




}




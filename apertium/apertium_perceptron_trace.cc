#include <fstream>
#include <string>

#include <apertium/mtx_reader.h>
#include <apertium/perceptron_tagger.h>
#include <apertium/shell_utils.h>
#include <apertium/stream_tagger.h>

namespace Apertium {

using namespace Apertium::ShellUtils;

int perceptron_trace(int argc, char* argv[]) 
{
  std::locale::global(std::locale(""));
  if (argc == 3 && std::string(argv[1]) == "model") 
  {
    // Dumps a tagger model
    basic_Tagger::Flags flags;
    PerceptronTagger pt(flags);
    std::ifstream tagger_model;
    try_open_fstream("MODEL", argv[2], tagger_model);
    pt.deserialise(tagger_model);
    std::wcout << pt;
  } 
  else if (argc == 3 && std::string(argv[1]) == "mtx") 
  {
    PerceptronSpec spec;
    MTXReader mtx_reader(spec);
    mtx_reader.read(argv[2]);
    std::wcout << "== Macro definitions ==\n";
    mtx_reader.printTmplDefns();
    std::wcout << "== Spec ==\n";
    std::wcout << spec;
  } 
  else if (argc == 5 && std::string(argv[1]) == "path") 
  {
    // Dumps features generated by every wordoid in every path using a correctly tagged path
    // Doesn't use a model
    basic_Tagger::Flags flags;
    PerceptronTagger pt(flags);
    pt.read_spec(argv[2]);

    std::wifstream untagged_stream;
    try_open_fstream("UNTAGGED_CORPUS", argv[3], untagged_stream);
    Stream untagged(flags, untagged_stream, argv[3]);

    std::wifstream tagged_stream;
    try_open_fstream("TAGGED_CORPUS", argv[4], tagged_stream);
    Stream tagged(flags, tagged_stream, argv[4]);

    TrainingCorpus tc(tagged, untagged, false, false);

    size_t sent_idx, token_idx, analy_idx, wrd_idx;
    for (sent_idx=0; sent_idx<tc.sentences.size(); sent_idx++) 
    {
      TaggedSentence &tagged_sent = tc.sentences[sent_idx].first;
      Sentence &untagged_sent = tc.sentences[sent_idx].second;
      for (token_idx=0; token_idx<untagged_sent.size(); token_idx++) 
      {
        if (!untagged_sent[token_idx].TheLexicalUnit) 
        {
          continue;
        }
        LexicalUnit &lu = *untagged_sent[token_idx].TheLexicalUnit;
        for (analy_idx=0; analy_idx<lu.TheAnalyses.size(); analy_idx++) 
        {
          Optional<Analysis> saved_token = tagged_sent[token_idx];
          tagged_sent[token_idx] = lu.TheAnalyses[analy_idx];
          std::vector<Morpheme> &wordoids = lu.TheAnalyses[analy_idx].TheMorphemes;
          for (wrd_idx=0; wrd_idx<wordoids.size(); wrd_idx++) 
          {
            UnaryFeatureVec feat_vec;
            pt.spec.get_features(
              tagged_sent, untagged_sent,
              token_idx, wrd_idx,
              feat_vec);
            std::wcout << "Sentence " << sent_idx << " of " << tc.sentences.size() << "\t\t"
                       << "Token " << token_idx << " of " << untagged_sent.size() << "\t\t"
                       << "Analysis " << analy_idx << " of " << lu.TheAnalyses.size() << "\t\t"
                       << "Wordoid " << wrd_idx << " of " << wordoids.size() << "\n";
            std::wcout << "" << wordoids[wrd_idx] << "\n";
            FeatureVec fv(feat_vec);
            std::wcout << fv;
            std::wcout << "\n\n";
          }
          tagged_sent[token_idx] = saved_token;
        }
      }
    }
  } 
  else 
  {
    std::wcout << "Run with one of:\n";
    std::wcout << argv[0] << " model <binary model file>\n";
    std::wcout << "Output features and weights from a model file.\n";
    std::wcout << argv[0] << " mtx <mtx file>\n";
    std::wcout << "Output macros and features from an mtx file.\n";
    std::wcout << argv[0] << " path <mtx file> <untagged> <tagged>\n";
    std::wcout << "Trace a particular path through giving which features fire "
               << "and the resulting score. Useful for interactively "
               << "designing feature sets.\n";
  }
  return 0;
}

}


int main(int argc, char* argv[]) {
  return Apertium::perceptron_trace(argc, argv);
}

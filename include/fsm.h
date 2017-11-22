#ifndef _FSM_H
#define _FSM_H

#include <unordered_map>
#include <vector>
#include <tuple>

namespace fsm
{
  //_Ind --- state index type
  //_Cond --- transition condition type
  //_Out --- output type
  template<typename _Ind, typename _Out, typename _Cond>
  class FSM
  {
    typedef _Ind index_type;
    typedef _Cond condition_type;
    typedef _Out output_type;

    typedef std::unordered_map<condition_type, index_type> tran_type;
    typedef std::pair<output_type, tran_type> state_type;

  private:
    std::unordered_map<index_type, state_type> _states;
    index_type _curr_state_index;
    std::pair<typename decltype(_states)::iterator, bool> _state_insert_result;
    std::pair<typename tran_type::iterator, bool> _tran_insert_result;
    
  public:
    FSM(){};
    ~FSM(){};
    void setState(_Ind index, _Out output)
    {
      state_type state_tmp = std::make_pair(output, tran_type());
      //Try to insert this new state.
      _state_insert_result = _states.insert(std::make_pair(index,state_tmp));

      //State index exists
      if(!_state_insert_result.second)
	{
	  state_type& state_ref = _state_insert_result.first->second;
	  //overwrite state output
	  state_ref.first = output;
	}

      //Set current state to the latest inserted.
      _curr_state_index = index;
    }
      
    int setTrans(_Ind src_index, _Cond condition, _Ind dst_index)
    {
      //src_index does not exist
      if(_states.find(src_index) == _states.end())
	return -1;

      //dst_index does not exist
      if(_states.find(dst_index) == _states.end())
	return -2;

      tran_type& tran_ref = _states[src_index].second;
      //Try to insert this transition.
      _tran_insert_result = tran_ref.insert(std::make_pair(condition, dst_index));
      //If a transition with the same condition exists, overwrite;
      if(!_tran_insert_result.second)
	_tran_insert_result.first->second = dst_index;

      return 0;
    }
    
    int transit(_Cond condition)
    {
      state_type& curr_state = _states[_curr_state_index];
      tran_type& curr_tran = curr_state.second;

    //Transition condition of current state is not found.
      if(curr_tran.find(condition) == curr_tran.end())
	return -1;
    //Transition condition of current state found.

      _curr_state_index = curr_tran[condition];
      return 0;
    }
    
    inline _Ind getCurrState() const
    {
      return _curr_state_index;
    }
    
    inline _Out getCurrOutput()
    {
      return _states[_curr_state_index].first;
    }
      
    bool setCurrState(_Ind index)
    {
      //index not found.
      if(_states.find(index) == _states.end())
	return false;
    
      _curr_state_index = index;
      return true;
    }
  };

}
#endif
